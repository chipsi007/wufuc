#include "stdafx.h"
#include "ptrlist.h"
#include "wufuc.h"
#include "hooks.h"
#include "log.h"
#include "modulehelper.h"
#include "mutexhelper.h"
#include "patternfind.h"
#include "resourcehelper.h"
#include "versionhelper.h"

#include <minhook.h>

HANDLE g_hMainMutex;

static bool close_remote_handle(HANDLE hProcess, HANDLE hObject)
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

static bool wufuc_get_patch_info(VS_FIXEDFILEINFO *pffi, PATCHINFO *ppi)
{
#ifdef _WIN64
        if ( ver_verify_version_info(6, 1, 1) && ver_compare_product_version(pffi, 7, 6, 7601, 23714) != -1
                || ver_verify_version_info(6, 3, 0) && ver_compare_product_version(pffi, 7, 9, 9600, 18621) != -1 ) {

                ppi->pattern = "FFF3 4883EC?? 33DB 391D???????? 7508 8B05????????";
                ppi->off1 = 0xa;
                ppi->off2 = 0x12;
                return true;
        }
#elif _WIN32
        if ( ver_verify_version_info(6, 1, 1)
                && ver_compare_product_version(pffi, 7, 6, 7601, 23714) != -1 ) {

                ppi->pattern = "833D????????00 743E E8???????? A3????????";
                ppi->off1 = 0x2;
                ppi->off2 = 0xf;
                return true;
        } else if ( ver_verify_version_info(6, 3, 0)
                && ver_compare_product_version(pffi, 7, 9, 9600, 18621) != -1 ) {

                ppi->pattern = "8BFF 51 833D????????00 7507 A1????????";
                ppi->off1 = 0x5;
                ppi->off2 = 0xd;
                return true;
        }
#endif
        return false;
}

static bool wufuc_get_patch_ptrs(const PATCHINFO *ppi, uintptr_t pfn, PBOOL *ppval1, PBOOL *ppval2)
{
#ifdef _WIN64
        *ppval1 = (PBOOL)(pfn + ppi->off1 + sizeof(uint32_t) + *(uint32_t *)(pfn + ppi->off1));
        *ppval2 = (PBOOL)(pfn + ppi->off2 + sizeof(uint32_t) + *(uint32_t *)(pfn + ppi->off2));
        return true;
#elif _WIN32
        *ppval1 = (PBOOL)(*(uintptr_t *)(pfn + ppi->off1));
        *ppval2 = (PBOOL)(*(uintptr_t *)(pfn + ppi->off2));
        return true;
#else
        return false;
#endif
}

void wufuc_patch(HMODULE hModule)
{
        void *pBlock;
        PATCHINFO pi;
        size_t count;
        PLANGANDCODEPAGE plcp;
        wchar_t *pInternalName;
        VS_FIXEDFILEINFO *pffi;
        MODULEINFO modinfo;
        size_t offset;
        void *pfn;
        DWORD fOldProtect;
        PBOOL pval1;
        PBOOL pval2;

        pBlock = res_get_version_info(hModule);
        if ( !pBlock ) return;

        plcp = res_query_var_file_info(pBlock, &count);
        if ( !plcp ) goto free_pBlock;

        for ( size_t i = 0; i < count; i++ ) {
                pInternalName = res_query_string_file_info(pBlock, plcp[i], L"InternalName", NULL);
                if ( pInternalName && !_wcsicmp(pInternalName, L"wuaueng.dll") )
                        goto cont_patch;
        }
        goto free_pBlock;

cont_patch:
        pffi = res_query_fixed_file_info(pBlock);
        if ( !pffi ) goto free_pBlock;

        if ( !wufuc_get_patch_info(pffi, &pi) ) {
                log_warning(L"Unsupported Windows Update Agent version: %hu.%hu.%hu.%hu",
                        HIWORD(pffi->dwProductVersionMS),
                        LOWORD(pffi->dwProductVersionMS),
                        HIWORD(pffi->dwProductVersionLS),
                        LOWORD(pffi->dwProductVersionLS));
                goto free_pBlock;
        }
        log_info(L"Supported Windows Update Agent version: %hu.%hu.%hu.%hu",
                HIWORD(pffi->dwProductVersionMS),
                LOWORD(pffi->dwProductVersionMS),
                HIWORD(pffi->dwProductVersionLS),
                LOWORD(pffi->dwProductVersionLS));

        if ( !GetModuleInformation(GetCurrentProcess(), hModule, &modinfo, sizeof modinfo) ) {
                log_error(L"GetModuleInformation failed! (hModule=%p, GLE=%lu)", hModule, GetLastError());
                goto free_pBlock;
        }
        offset = patternfind(modinfo.lpBaseOfDll, modinfo.SizeOfImage, pi.pattern);
        if ( offset == -1 ) {
                log_info(L"Couldn't match IsDeviceServiceable function!");
                goto free_pBlock;
        }
        pfn = OffsetToPointer(modinfo.lpBaseOfDll, offset);
        log_info(L"Matched %ls!IsDeviceServiceable function! (Offset=%IX, Address=%p)",
                PathFindFileNameW(g_pszWUServiceDll), offset, pfn);

        if ( wufuc_get_patch_ptrs(&pi, (uintptr_t)pfn, &pval1, &pval2) ) {
                if ( *pval1 && VirtualProtect(pval1, sizeof *pval1, PAGE_READWRITE, &fOldProtect) ) {
                        *pval1 = FALSE;
                        VirtualProtect(pval1, sizeof *pval1, fOldProtect, &fOldProtect);
                        log_info(L"Patched variable! (Address=%p)", pval1);
                }
                if ( !*pval2 && VirtualProtect(pval2, sizeof *pval2, PAGE_READWRITE, &fOldProtect) ) {
                        *pval2 = TRUE;
                        VirtualProtect(pval2, sizeof *pval2, fOldProtect, &fOldProtect);
                        log_info(L"Patched variable! (Address=%p)", pval2);
                }
        }
free_pBlock:
        free(pBlock);
}
