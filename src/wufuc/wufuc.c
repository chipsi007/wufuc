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
                log_info(L"Successfully injected into process! (ProcessId=%lu)", dwProcessId);
        else
                log_warning(L"Failed to inject into process! (ProcessId=%lu)", dwProcessId);
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
        bool tmp;
        MODULEINFO modinfo;
        size_t offset;
        LPVOID pTarget = NULL;
        MH_STATUS status;

        ptl = ver_get_version_info_from_hmodule_alloc(hModule, L"\\VarFileInfo\\Translation", &cbtl);
        if ( !ptl ) {
                log_error(L"ver_get_version_info_from_hmodule_alloc failed!");
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
                        log_error(L"ver_get_version_info_from_hmodule_alloc failed!");
                        continue;
                }
                // identify wuaueng.dll by its resource data
                if ( _wcsicmp(pInternalName, L"wuaueng.dll") ) {
                        log_error(L"Module internal name does not match! (InternalName=%ls)", pInternalName);
                        goto free_iname;
                }
                pffi = ver_get_version_info_from_hmodule_alloc(hModule, L"\\", &cbffi);
                if ( !pffi ) {
                        log_error(L"ver_get_version_info_from_hmodule_alloc failed!");
                        break;
                }
                // assure wuaueng.dll version is supported
                tmp = ((ver_verify_version_info(6, 1, 0) && ver_compare_product_version(pffi, 7, 6, 7601, 23714) != -1)
                        || (ver_verify_version_info(6, 3, 0) && ver_compare_product_version(pffi, 7, 9, 9600, 18621) != -1));

                log_info(L"%ls Windows Update Agent version: %hu.%hu.%hu.%hu",
                        tmp ? L"Supported" : L"Unsupported",
                        HIWORD(pffi->dwProductVersionMS),
                        LOWORD(pffi->dwProductVersionMS),
                        HIWORD(pffi->dwProductVersionLS),
                        LOWORD(pffi->dwProductVersionLS));
                free(pffi);
                if ( !tmp ) break;

                if ( !GetModuleInformation(hProcess, hModule, &modinfo, sizeof modinfo) ) {
                        log_error(L"GetModuleInformation failed! (hModule=%p, GLE=%lu)",
                                hModule, GetLastError());
                        break;
                }
                offset = patternfind(modinfo.lpBaseOfDll, modinfo.SizeOfImage,
#ifdef _WIN64
                        "FFF3 4883EC?? 33DB 391D???????? 7508 8B05????????"
#else
                        ver_verify_version_info(6, 1, 0)
                        ? "833D????????00 743E E8???????? A3????????"
                        : "8BFF 51 833D????????00 7507 A1????????"
#endif
                );
                if ( offset != -1 ) {
                        pTarget = (LPVOID)RtlOffsetToPointer(modinfo.lpBaseOfDll, offset);
                        log_info(L"Matched IsDeviceServiceable function! (Offset=%IX, Address=%p)", offset, pTarget);

                        status = MH_CreateHook(pTarget, IsDeviceServiceable_hook, NULL);
                        if ( status == MH_OK ) {
                                status = MH_EnableHook(pTarget);
                                if ( status == MH_OK )
                                        log_info(L"Hooked IsDeviceServiceable! (Address=%p)", pTarget);
                                else log_error(L"Failed to enable IsDeviceServiceable hook! (Status=%hs)", MH_StatusToString(status));
                        } else log_error(L"Failed to create IsDeviceServiceable hook! (Status=%hs)", MH_StatusToString(status));
                } else log_info(L"Couldn't match IsDeviceServiceable function! (Already patched?)");
free_iname:
                free(pInternalName);
                break;
        }
        free(ptl);
        return result;
}
