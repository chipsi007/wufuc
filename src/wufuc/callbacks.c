#include "stdafx.h"
#include "callbacks.h"
#include "hooks.h"
#include "log.h"
#include "modulehelper.h"
#include "registryhelper.h"
#include "servicehelper.h"
#include "versionhelper.h"
#include "ptrlist.h"
#include "wufuc.h"

#include <VersionHelpers.h>
#include <minhook.h>

VOID CALLBACK service_notify_callback(PSERVICE_NOTIFYW pNotifyBuffer)
{
        switch ( pNotifyBuffer->dwNotificationStatus ) {
        case ERROR_SUCCESS:
                if ( pNotifyBuffer->ServiceStatus.dwProcessId )
                        wufuc_inject(
                                pNotifyBuffer->ServiceStatus.dwProcessId,
                                (LPTHREAD_START_ROUTINE)thread_start_callback,
                                (ptrlist_t *)pNotifyBuffer->pContext);
                break;
        case ERROR_SERVICE_MARKED_FOR_DELETE:
                SetEvent(ptrlist_at((ptrlist_t *)pNotifyBuffer->pContext, 0, NULL));
                break;
        }
        if ( pNotifyBuffer->pszServiceNames )
                LocalFree((HLOCAL)pNotifyBuffer->pszServiceNames);
}

DWORD WINAPI thread_start_callback(HANDLE *pParam)
{
        HANDLE handles[2];
        HANDLE hCrashMutex;
        HANDLE hProceedEvent;
        SC_HANDLE hSCM;
        SC_HANDLE hService;
        DWORD dwProcessId;
        LPQUERY_SERVICE_CONFIGW pServiceConfig;
        DWORD dwServiceType;
        const wchar_t szKernel32Dll[] = L"kernel32.dll";
        const wchar_t szKernelBaseDll[] = L"kernelbase.dll";
        const wchar_t *pszModule;
        MH_STATUS status;
        int tmp;
        LPVOID pv1 = NULL;
        LPVOID pv2 = NULL;
        wchar_t *str;
        HMODULE hModule;

        if ( !pParam ) {
                log_error(L"Parameter argument is null!");
                goto unload;
        }
        handles[0] = pParam[0]; // main mutex
        handles[1] = pParam[1]; // unload event
        hCrashMutex = pParam[2]; // crash mutex
        hProceedEvent = pParam[3]; // proceed event
        if ( !VirtualFree(pParam, 0, MEM_RELEASE) )
                log_warning(L"VirtualFree failed! (lpAddress=%p, GLE=%lu)", pParam, GetLastError());

        // acquire child mutex, this should be immediate.
        if ( WaitForSingleObject(hCrashMutex, 5000) != WAIT_OBJECT_0 ) {
                log_error(L"Failed to acquire crash mutex within five seconds. (%p)", hCrashMutex);
                goto close_handles;
        }
        SetEvent(hProceedEvent);
        CloseHandle(hProceedEvent);

        hSCM = OpenSCManagerW(NULL, NULL, SC_MANAGER_CONNECT);
        if ( !hSCM ) {
                log_error(L"Failed to open SCM. (GetLastError=%lu)", GetLastError());
                goto release;
        }
        hService = OpenServiceW(hSCM, L"wuauserv", SERVICE_QUERY_STATUS | SERVICE_QUERY_CONFIG);
        dwProcessId = svc_heuristic_process_id(hSCM, hService);
        pServiceConfig = svc_query_config_alloc(hSCM, hService, NULL);
        dwServiceType = pServiceConfig->dwServiceType;
        tmp = _wcsicmp(pServiceConfig->lpBinaryPathName, GetCommandLineW());
        free(pServiceConfig);
        CloseServiceHandle(hService);
        CloseServiceHandle(hSCM);

        if ( tmp || dwProcessId != GetCurrentProcessId() ) {
                log_error(L"Injected into wrong process!");
                goto release;
        }
        if ( !ver_verify_version_info(6, 1, 0) && !ver_verify_version_info(6, 3, 0) ) {
                log_error(L"Unsupported operating system!");
                goto release;
        }
        if ( dwServiceType == SERVICE_WIN32_SHARE_PROCESS ) {
                // assume wuaueng.dll hasn't been loaded yet, apply
                // RegQueryValueExW hook to fix incompatibility with
                // UpdatePack7R2 and other patches that modify the
                // Windows Update ServiceDll path in the registry.
                pszModule = IsWindows8OrGreater()
                        ? szKernelBaseDll
                        : szKernel32Dll;
                status = MH_CreateHookApiEx(pszModule,
                        "RegQueryValueExW",
                        RegQueryValueExW_hook,
                        &(PVOID)g_pfnRegQueryValueExW,
                        &pv1);
                if ( status == MH_OK ) {
                        status = MH_EnableHook(pv1);
                        if ( status == MH_OK )
                                log_info(L"Hooked RegQueryValueExW! (Module=%ls, Address=%p)", pszModule, pv1);
                        else log_error(L"Failed to enable RegQueryValueExW hook! (Status=%hs)", MH_StatusToString(status));
                } else log_error(L"Failed to create RegQueryValueExW hook! (Status=%hs)", MH_StatusToString(status));
        }
        // query the ServiceDll path after applying our compat hook so that it
        // is consistent
        str = (wchar_t *)reg_query_value_alloc(HKEY_LOCAL_MACHINE,
                L"SYSTEM\\CurrentControlSet\\services\\wuauserv\\Parameters",
                L"ServiceDll", NULL, NULL);
        if ( !str ) {
abort_hook:
                if ( pv1 )
                        MH_RemoveHook(pv1);
                goto release;
        }
        g_pszWUServiceDll = env_expand_strings_alloc(str, NULL);
        free(str);
        if ( !g_pszWUServiceDll ) goto abort_hook;

        status = MH_CreateHookApiEx(szKernelBaseDll,
                "LoadLibraryExW",
                LoadLibraryExW_hook,
                &(PVOID)g_pfnLoadLibraryExW,
                &pv2);
        if ( status == MH_OK ) {
                status = MH_EnableHook(pv2);
                if ( status == MH_OK )
                        log_info(L"Hooked LoadLibraryExW! (Module=%ls, Address=%p)", szKernelBaseDll, pv2);
                else log_error(L"Failed to enable LoadLibraryExW hook! (Status=%hs)", MH_StatusToString(status));
        } else log_error(L"Failed to create LoadLibraryExW hook! (Status=%hs)", MH_StatusToString(status));

        if ( GetModuleHandleExW(0, g_pszWUServiceDll, &hModule)
                || GetModuleHandleExW(0, PathFindFileNameW(g_pszWUServiceDll), &hModule) ) {

                // hook IsDeviceServiceable if wuaueng.dll is already loaded
                wufuc_patch(hModule);
                FreeLibrary(hModule);
        }
        // wait for unload event or the main mutex to be released or abandoned,
        // for example if the user killed rundll32.exe with task manager.
        WaitForMultipleObjects(_countof(handles), handles, FALSE, INFINITE);
        log_info(L"Unload condition has been met.");

        MH_DisableHook(MH_ALL_HOOKS);
        free(g_pszWUServiceDll);
release:
        ReleaseMutex(hCrashMutex);
close_handles:
        CloseHandle(hCrashMutex);
        CloseHandle(handles[0]);
        CloseHandle(handles[1]);
unload:
        log_info(L"Unloading wufuc and exiting thread.");
        FreeLibraryAndExitThread(PIMAGEBASE, 0);
}

