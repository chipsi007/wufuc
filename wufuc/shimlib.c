#include "shimlib.h"
#include "hooks.h"

#include <Windows.h>

PHOOKAPI g_pHookApiArray;

PHOOKAPI WINAPI GetHookAPIs(LPCSTR szCommandLine, LPCWSTR wszShimName, PDWORD pdwHookCount) {
    g_pHookApiArray = calloc(2, sizeof(HOOKAPI));

    if ( g_pHookApiArray ) {
        g_pHookApiArray[0].LibraryName = "advapi32.dll";
        g_pHookApiArray[0].FunctionName = "RegQueryValueExW";
        g_pHookApiArray[0].ReplacementFunction = RegQueryValueExW_hook;
        g_pHookApiArray[1].LibraryName = "kernel32.dll";
        g_pHookApiArray[1].FunctionName = "LoadLibraryExW";
        g_pHookApiArray[1].ReplacementFunction = LoadLibraryExW_hook;
        *pdwHookCount = 2;
    }
    return NULL;
}

BOOL WINAPI NotifyShims(DWORD fdwReason, PLDR_DATA_TABLE_ENTRY pLdrEntry) {
    switch ( fdwReason ) {
    case SHIM_NOTIFY_ATTACH:
        break;
    case SHIM_NOTIFY_DETACH:
        break;
    case SHIM_NOTIFY_DLL_LOAD:
        break;
    case SHIM_NOTIFY_DLL_UNLOAD:
        break;
    }
    return TRUE;
}
