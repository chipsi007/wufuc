#include "ntdllhelper.h"

#define WIN32_NO_STATUS
#include <windows.h>
#undef WIN32_NO_STATUS

#include <winternl.h>
#include <tchar.h>

static HMODULE g_hNTDLL = NULL;

static BOOL InitNTDLL(void) {
    if ( !g_hNTDLL )
        g_hNTDLL = GetModuleHandle(_T("ntdll"));
    return !!g_hNTDLL;
}

BOOL TryNtQueryKey(HANDLE KeyHandle, KEY_INFORMATION_CLASS KeyInformationClass, PVOID KeyInformation, ULONG Length, PULONG ResultLength, PNTSTATUS status) {
    static NTQUERYKEY pfnNtQueryKey = NULL;

    if ( InitNTDLL() ) {
        if ( !pfnNtQueryKey )
            pfnNtQueryKey = (NTQUERYKEY)GetProcAddress(g_hNTDLL, "NtQueryKey");

        if ( pfnNtQueryKey ) {
            *status = pfnNtQueryKey(KeyHandle, KeyInformationClass, KeyInformation, Length, ResultLength);
            return TRUE;
        }
    }
    return FALSE;
}
