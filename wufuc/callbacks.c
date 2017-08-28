#include "callbacks.h"

#include "service.h"
#include "iathook.h"
#include "hooks.h"
#include "patchwua.h"
#include "helpers.h"
#include "ntdllhelper.h"
#include "logging.h"

#include <stdint.h>

#define WIN32_NO_STATUS
#include <windows.h>
#undef WIN32_NO_STATUS

#include <winternl.h>
#include <tchar.h>
#include <Psapi.h>
#include <sddl.h>

DWORD WINAPI ThreadProcCallback(LPVOID lpParam) {
    TCHAR lpServiceCommandLine[0x8000];
    GetServiceCommandLine(NULL, _T("wuauserv"), lpServiceCommandLine, _countof(lpServiceCommandLine));
    if ( _tcsicmp(GetCommandLine(), lpServiceCommandLine) )
        return 0;

    SECURITY_ATTRIBUTES sa = { 0 };
    sa.nLength = sizeof(SECURITY_ATTRIBUTES);
    sa.bInheritHandle = FALSE;
    if ( !ConvertStringSecurityDescriptorToSecurityDescriptor(_T("D:(A;;0x001F0003;;;BA)"), SDDL_REVISION_1, &sa.lpSecurityDescriptor, NULL) ) {
        trace(_T("Failed to convert string security descriptor to security descriptor (error code=%08X)"), GetLastError());
        return 0;
    }

    HANDLE hEvent = CreateEvent(&sa, TRUE, FALSE, _T("Global\\wufuc_UnloadEvent"));
    if ( hEvent ) {
        iat_hook(GetModuleHandle(NULL), "LoadLibraryExW", (LPVOID)&fpLoadLibraryExW, LoadLibraryExW_hook);

        PVOID cookie;
        NTSTATUS status;
        if ( TryLdrRegisterDllNotification(0, (PLDR_DLL_NOTIFICATION_FUNCTION)LdrDllNotificationCallback, NULL, &cookie, &status) ) {
            if ( NT_SUCCESS(status) ) {
                trace(_T("Registered LdrDllNotificationCallback (status code=%d)"), status);
                HMODULE hm = GetModuleHandleW(GetWindowsUpdateServiceDllW());
                if ( hm ) {
                    MODULEINFO modinfo;
                    if ( GetModuleInformation(GetCurrentProcess(), hm, &modinfo, sizeof(MODULEINFO)) )
                        PatchWUA((void *)modinfo.lpBaseOfDll, (size_t)modinfo.SizeOfImage);
                }

                WaitForSingleObject(hEvent, INFINITE);
                trace(_T("Unload event was set"));

                if ( cookie ) {
                    if ( TryLdrUnregisterDllNotification(cookie, &status) ) {
                        if ( NT_SUCCESS(status) )
                            trace(_T("Unregistered LdrDllNotificationCallback (status code=%d)"), status);
                        else
                            trace(_T("Failed to unregister LdrDllNotificationCallback (status code=%d)"), status);
                    } else
                        trace(_T("Failed to get LdrUnregisterDllNotification proc address"));
                }
            } else
                trace(_T("Failed to register LdrDllNotificationCallback (status code=%d)"), status);
        } else
            trace(_T("Failed to get LdrRegisterDllNotification proc address"));

        iat_hook(GetModuleHandle(NULL), "LoadLibraryExW", NULL, fpLoadLibraryExW);
        CloseHandle(hEvent);
    } else {
        trace(_T("Failed to create unload event (error code=%08X)"), GetLastError());
        return 0;
    }
    trace(_T("Bye bye!"));
    FreeLibraryAndExitThread((HMODULE)lpParam, 0);
}

VOID CALLBACK LdrDllNotificationCallback(ULONG NotificationReason, PCLDR_DLL_NOTIFICATION_DATA NotificationData, PVOID Context) {
    switch ( NotificationReason ) {
    case LDR_DLL_NOTIFICATION_REASON_LOADED:
    {
        LDR_DLL_LOADED_NOTIFICATION_DATA data = NotificationData->Loaded;

        trace(_T("Loaded library: %wZ"), data.FullDllName);

        wchar_t buffer[MAX_PATH];
        wcscpy_s(buffer, _countof(buffer), (wchar_t *)GetWindowsUpdateServiceDllW());
        ApplyUpdatePack7R2ShimIfNeeded(buffer, _countof(buffer), buffer, _countof(buffer));
        
        if ( !_wcsnicmp((wchar_t *)data.FullDllName->Buffer, buffer, (size_t)data.FullDllName->Length) )
            PatchWUA(data.DllBase, data.SizeOfImage);
        return;
    }
    case LDR_DLL_NOTIFICATION_REASON_UNLOADED:
    {
        LDR_DLL_UNLOADED_NOTIFICATION_DATA data = NotificationData->Unloaded;

        trace(_T("Unloaded library: %wZ"), data.FullDllName);
        return;
    }
    default:
        return;
    }
}

