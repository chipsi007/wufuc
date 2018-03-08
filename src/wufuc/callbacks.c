#include "stdafx.h"
#include "callbacks.h"
#include "hooks.h"
#include "log.h"
#include "modulehelper.h"
#include "registryhelper.h"
#include "servicehelper.h"
#include "ptrlist.h"
#include "wufuc.h"

#include <minhook.h>

VOID CALLBACK cb_service_notify(PSERVICE_NOTIFYW pNotifyBuffer)
{
        trace(L"enter");
        switch ( pNotifyBuffer->dwNotificationStatus ) {
        case ERROR_SUCCESS:
                if ( pNotifyBuffer->ServiceStatus.dwProcessId )
                        wufuc_inject(
                                pNotifyBuffer->ServiceStatus.dwProcessId,
                                (LPTHREAD_START_ROUTINE)cb_start,
                                (ptrlist_t *)pNotifyBuffer->pContext);
                break;
        case ERROR_SERVICE_MARKED_FOR_DELETE:
                SetEvent(ptrlist_at((ptrlist_t *)pNotifyBuffer->pContext, 1, NULL));
                break;
        }
        if ( pNotifyBuffer->pszServiceNames )
                LocalFree((HLOCAL)pNotifyBuffer->pszServiceNames);
}

DWORD WINAPI cb_start(HANDLE *pParam)
{
        HANDLE handles[2];
        HANDLE hCrashMutex;
        HANDLE hProceedEvent;
        SC_HANDLE hSCM;
        SC_HANDLE hService;
        DWORD dwProcessId;
        LPQUERY_SERVICE_CONFIGW pServiceConfig;
        DWORD dwServiceType;
        LPVOID pTarget = NULL;
        wchar_t *str;
        HMODULE hModule;

        if ( !pParam ) {
                trace(L"Parameter argument is null!");
                goto unload;
        }

        handles[0] = pParam[0]; // main mutex
        handles[1] = pParam[1]; // unload event
        hCrashMutex = pParam[2]; // crash mutex
        hProceedEvent = pParam[3]; // proceed event
        VirtualFree(pParam, 0, MEM_RELEASE);

        // acquire child mutex, this should be immediate.
        if ( WaitForSingleObject(hCrashMutex, 5000) != WAIT_OBJECT_0 ) {
                trace(L"Failed to acquire child mutex within five seconds. (%p)", hCrashMutex);
                goto close_handles;
        }
        SetEvent(hProceedEvent);

        hSCM = OpenSCManagerW(NULL, NULL, SC_MANAGER_CONNECT);
        if ( !hSCM ) {
                trace(L"Failed to open SCM. (GetLastError=%lu)", GetLastError());
                goto release;
        }

        hService = OpenServiceW(hSCM, L"wuauserv",
                SERVICE_QUERY_STATUS | SERVICE_QUERY_CONFIG);
        dwProcessId = svc_heuristic_process_id(hSCM, hService);
        pServiceConfig = svc_query_config_alloc(hSCM, hService, NULL);
        dwServiceType = pServiceConfig->dwServiceType;
        free(pServiceConfig);
        CloseServiceHandle(hService);
        CloseServiceHandle(hSCM);

        if ( dwProcessId != GetCurrentProcessId() ) {
                trace(L"Injected into wrong process!");
                goto release;
        }

        if ( dwServiceType == SERVICE_WIN32_SHARE_PROCESS ) {
                // assume wuaueng.dll hasn't been loaded yet, apply
                // RegQueryValueExW hook to fix incompatibility with
                // UpdatePack7R2 and other patches that modify the
                // Windows Update ServiceDll path in the registry.
                trace(L"MH_CreateHookApi RegQueryValueExW=%hs", MH_StatusToString(MH_CreateHookApiEx(L"kernel32.dll",
                        "RegQueryValueExW",
                        RegQueryValueExW_hook,
                        &(PVOID)g_pfnRegQueryValueExW,
                        &pTarget)));
                trace(L"MH_EnableHook RegQueryValueExW=%hs", MH_StatusToString(MH_EnableHook(pTarget)));
        }

        // query the ServiceDll path after applying our compat hook so that it
        // is correct
        str = (wchar_t *)reg_query_value_alloc(HKEY_LOCAL_MACHINE,
                L"SYSTEM\\CurrentControlSet\\services\\wuauserv\\Parameters",
                L"ServiceDll", NULL, NULL);
        if ( !str ) {
abort_hook:
                if ( pTarget )
                        trace(L"MH_RemoveHook RegQueryValueExW=%hs", MH_StatusToString(MH_RemoveHook(pTarget)));
                goto release;
        }
        g_pszWUServiceDll = env_expand_strings_alloc(str, NULL);
        free(str);
        if ( !g_pszWUServiceDll ) goto abort_hook;

        trace(L"MH_CreateHookApi LoadLibraryExW=%hs", MH_StatusToString(MH_CreateHookApi(L"kernel32.dll",
                "LoadLibraryExW",
                LoadLibraryExW_hook,
                &(PVOID)g_pfnLoadLibraryExW)));

        if ( GetModuleHandleExW(0, g_pszWUServiceDll, &hModule)
                || GetModuleHandleExW(0, PathFindFileNameW(g_pszWUServiceDll), &hModule) ) {

                // hook IsDeviceServiceable if wuaueng.dll is already loaded
                wufuc_hook(hModule);
                FreeLibrary(hModule);

        }
        trace(L"MH_EnableHook=%hs", MH_StatusToString(MH_EnableHook(MH_ALL_HOOKS)));

        // wait for unload event or the main mutex to be released or abandoned,
        // for example if the user killed rundll32.exe with task manager.
        WaitForMultipleObjects(_countof(handles), handles, FALSE, INFINITE);
        trace(L"Unload condition has been met.");

        trace(L"MH_DisableHook(MH_ALL_HOOKS) LoadLibraryExW=%hs", MH_StatusToString(MH_DisableHook(MH_ALL_HOOKS)));
        free(g_pszWUServiceDll);
release:
        ReleaseMutex(hCrashMutex);
close_handles:
        CloseHandle(hCrashMutex);
        CloseHandle(handles[0]);
        CloseHandle(handles[1]);
unload:
        trace(L"Unloading wufuc and exiting thread.");
        FreeLibraryAndExitThread(PIMAGEBASE, 0);
}

