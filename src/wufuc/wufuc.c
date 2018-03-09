#include "stdafx.h"
#include "ptrlist.h"
#include "wufuc.h"
#include "hooks.h"
#include "log.h"
#include "modulehelper.h"
#include "mutexhelper.h"
#include "patternfind.h"
#include "versionhelper.h"

#include <minhook.h>

HANDLE g_hMainMutex;

bool close_remote_handle(HANDLE hProcess, HANDLE hObject)
{
        bool result = false;
        DWORD ExitCode;
        HANDLE hThread;

        hThread = CreateRemoteThread(hProcess,
                NULL,
                0,
                (LPTHREAD_START_ROUTINE)CloseHandle,
                (LPVOID)hObject,
                0,
                NULL);
        if ( hThread ) {
                if ( WaitForSingleObject(hThread, INFINITE) == WAIT_OBJECT_0
                        && GetExitCodeThread(hThread, &ExitCode) ) {

                        result = !!ExitCode;
                }
                CloseHandle(hThread);
        }
        return result;
}

bool wufuc_inject(DWORD dwProcessId,
        LPTHREAD_START_ROUTINE pStartAddress,
        ptrlist_t *list)
{
        bool result = false;
        HANDLE hCrashMutex;
        HANDLE hProcess;
        HANDLE h;
        HANDLE hProceedEvent;
        HANDLE p[4];

        hCrashMutex = mutex_create_new_fmt(false, L"Global\\wufuc_CrashMutex*%08x", dwProcessId);
        if ( !hCrashMutex ) return result;
        if ( !ptrlist_add(list, hCrashMutex, dwProcessId) )
                goto close_mutex;

        hProceedEvent = CreateEventW(NULL, TRUE, FALSE, NULL);
        if ( !hProceedEvent ) goto close_mutex;

        hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwProcessId);
        if ( !hProcess ) goto close_pevent;

        h = GetCurrentProcess();
        if ( !DuplicateHandle(h, g_hMainMutex, hProcess, &p[0], SYNCHRONIZE, FALSE, 0) )
                goto close_process;
        if ( !DuplicateHandle(h, ptrlist_at(list, 0, NULL), hProcess, &p[1], SYNCHRONIZE, FALSE, 0) )
                goto close_p0;
        if ( !DuplicateHandle(h, hCrashMutex, hProcess, &p[2], 0, FALSE, DUPLICATE_SAME_ACCESS) )
                goto close_p1;
        if ( !DuplicateHandle(h, hProceedEvent, hProcess, &p[3], EVENT_MODIFY_STATE, FALSE, 0) )
                goto close_p2;

        result = mod_inject_and_begin_thread(hProcess, PIMAGEBASE, pStartAddress, p, sizeof p);

        if ( result ) {
                // wait for injected thread to signal that it has taken
                // ownership of hCrashMutex before proceeding.
                result = WaitForSingleObject(hProceedEvent, 5000) != WAIT_TIMEOUT;
        } else {
                close_remote_handle(hProcess, p[3]);
close_p2:
                close_remote_handle(hProcess, p[2]);
close_p1:
                close_remote_handle(hProcess, p[1]);
close_p0:
                close_remote_handle(hProcess, p[0]);
        }
close_process:
        CloseHandle(hProcess);
close_pevent:
        CloseHandle(hProceedEvent);
        if ( !result ) {
close_mutex:
                ptrlist_remove(list, hCrashMutex);
                CloseHandle(hCrashMutex);
        }
        if ( result )
                trace(L"Successfully injected into process: %lu", dwProcessId);
        return result;
}

bool wufuc_hook(HMODULE hModule)
{
        bool result = false;
        PLANGANDCODEPAGE ptl;
        HANDLE hProcess;
        UINT cbtl;
        wchar_t SubBlock[38];
        wchar_t *pInternalName;
        UINT cbInternalName;
        VS_FIXEDFILEINFO *pffi;
        UINT cbffi;
        int tmp;
        MODULEINFO modinfo;
        size_t offset;
        LPVOID pTarget;

        if ( !ver_verify_windows_7_sp1() && !ver_verify_windows_8_1() )
                return false;

        ptl = ver_get_version_info_from_hmodule_alloc(hModule, L"\\VarFileInfo\\Translation", &cbtl);
        if ( !ptl ) {
                trace(L"Failed to get translation info from hModule.");
                return false;
        }
        hProcess = GetCurrentProcess();

        for ( size_t i = 0, count = (cbtl / sizeof *ptl); i < count; i++ ) {
                if ( swprintf_s(SubBlock,
                        _countof(SubBlock),
                        L"\\StringFileInfo\\%04x%04x\\InternalName",
                        ptl[i].wLanguage,
                        ptl[i].wCodePage) == -1 )
                        continue;

                pInternalName = ver_get_version_info_from_hmodule_alloc(hModule, SubBlock, &cbInternalName);
                if ( !pInternalName ) {
                        trace(L"Failed to get internal name from hModule.");
                        continue;
                }

                // identify wuaueng.dll by its resource data
                if ( _wcsicmp(pInternalName, L"wuaueng.dll") ) {
                        trace(L"Module internal name does not match. (%ls)", pInternalName);
                        goto free_iname;
                }
                pffi = ver_get_version_info_from_hmodule_alloc(hModule, L"\\", &cbffi);
                if ( !pffi ) {
                        trace(L"Failed to get version info from hModule.");
                        break;
                }
                trace(L"Windows Update Agent version: %hu.%hu.%hu.%hu",
                        HIWORD(pffi->dwProductVersionMS),
                        LOWORD(pffi->dwProductVersionMS),
                        HIWORD(pffi->dwProductVersionLS),
                        LOWORD(pffi->dwProductVersionLS));

                // assure wuaueng.dll is at least the minimum supported version
                tmp = ((ver_verify_windows_7_sp1() && ver_compare_product_version(pffi, 7, 6, 7601, 23714) != -1)
                        || (ver_verify_windows_8_1() && ver_compare_product_version(pffi, 7, 9, 9600, 18621) != -1));
                free(pffi);
                if ( !tmp ) {
                        trace(L"Windows Update Agent does not meet the minimum supported version.");
                        break;
                }
                if ( !GetModuleInformation(hProcess, hModule, &modinfo, sizeof modinfo) ) {
                        trace(L"Failed to get module info: %p, %p (GLE=%08x)", hProcess, hModule, GetLastError());
                        break;
                }
                offset = patternfind(modinfo.lpBaseOfDll, modinfo.SizeOfImage,
#ifdef _WIN64
                        "FFF3 4883EC?? 33DB 391D???????? 7508 8B05????????"
#else
                        ver_verify_windows_7_sp1()
                        ? "833D????????00 743E E8???????? A3????????"
                        : "8BFF 51 833D????????00 7507 A1????????"
#endif
                );
                if ( offset != -1 ) {
                        pTarget = (LPVOID)RtlOffsetToPointer(modinfo.lpBaseOfDll, offset);
                        trace(L"Found IsDeviceServiceable function: %p", pTarget);

                        result = (MH_CreateHook(pTarget, IsDeviceServiceable_hook, NULL) == MH_OK)
                                && (MH_EnableHook(pTarget) == MH_OK);
                        if ( result )
                                trace(L"Successfully hooked IsDeviceServiceable!");
                } else {
                        trace(L"Could not find function offset!");
                }
free_iname:
                free(pInternalName);
                break;
        }
        free(ptl);
        return result;
}
