#ifndef SHIM_H
#define SHIM_H
#pragma once

#define WIN32_NO_STATUS
#include <windows.h>
#undef WIN32_NO_STATUS

#include <winternl.h>

typedef struct tagHOOKAPI {
    PCSTR LibraryName;
    PCSTR FunctionName;
    PVOID ReplacementFunction;
    PVOID OriginalFunction;
    PVOID Reserved[2];
} HOOKAPI, *PHOOKAPI;

#define SHIM_REASON_INIT       100
#define SHIM_REASON_DEINIT     101
#define SHIM_REASON_DLL_LOAD   102 /* Arg: PLDR_DATA_TABLE_ENTRY */
#define SHIM_REASON_DLL_UNLOAD 103 /* Arg: PLDR_DATA_TABLE_ENTRY */

#define SHIM_NOTIFY_ATTACH     1
#define SHIM_NOTIFY_DETACH     2
#define SHIM_NOTIFY_DLL_LOAD   3   /* Arg: PLDR_DATA_TABLE_ENTRY */
#define SHIM_NOTIFY_DLL_UNLOAD 4   /* Arg: PLDR_DATA_TABLE_ENTRY */

extern PHOOKAPI g_pHookApiArray;

PHOOKAPI WINAPI GetHookAPIs(LPCSTR szCommandLine, LPCWSTR wszShimName, PDWORD pdwHookCount);
BOOL WINAPI NotifyShims(DWORD fdwReason, PLDR_DATA_TABLE_ENTRY pLdrEntry);
#endif
