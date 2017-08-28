#include "ntdllhelper.h"

#define WIN32_NO_STATUS
#include <windows.h>
#undef WIN32_NO_STATUS

#include <winternl.h>
#include <tchar.h>

HMODULE g_hNTDLL = NULL;

BOOL InitNTDLL(void) {
    if ( !g_hNTDLL )
        g_hNTDLL = LoadLibrary(_T("ntdll"));
    return (g_hNTDLL != NULL);
}

BOOL FreeNTDLL(void) {
    if ( g_hNTDLL ) {
        BOOL result = FreeLibrary(g_hNTDLL);
        if ( result )
            g_hNTDLL = NULL;
        return result;
    }
    return FALSE;
}

BOOL TryLdrRegisterDllNotification(ULONG Flags, PLDR_DLL_NOTIFICATION_FUNCTION NotificationFunction, PVOID Context, PVOID *Cookie, NTSTATUS *result) {
    static LDRREGISTERDLLNOTIFICATION fpLdrRegisterDllNotification = NULL;

    if ( InitNTDLL() ) {
        if ( !fpLdrRegisterDllNotification )
            fpLdrRegisterDllNotification = (LDRREGISTERDLLNOTIFICATION)GetProcAddress(g_hNTDLL, "LdrRegisterDllNotification");
        
        if ( fpLdrRegisterDllNotification ) {
            *result = fpLdrRegisterDllNotification(Flags, NotificationFunction, Context, Cookie);
            return TRUE;
        }
    }
    return FALSE;
}

BOOL TryLdrUnregisterDllNotification(PVOID Cookie, NTSTATUS *result) {
    static LDRUNREGISTERDLLNOTIFICATION fpLdrUnregisterDllNotification = NULL;

    if ( InitNTDLL() ) {
        if ( !fpLdrUnregisterDllNotification )
            fpLdrUnregisterDllNotification = (LDRUNREGISTERDLLNOTIFICATION)GetProcAddress(g_hNTDLL, "LdrUnregisterDllNotification");

        if ( fpLdrUnregisterDllNotification ) {
            *result = fpLdrUnregisterDllNotification(Cookie);
            return TRUE;
        }
    }
    return FALSE;
}
