#include "hooks.h"

#include "service.h"
#include "helpers.h"
//#include "ntdllhelper.h"
#include "logging.h"

#include <stdint.h>

//#define WIN32_NO_STATUS
#include <Windows.h>
//#undef WIN32_NO_STATUS

//#include <winternl.h>
//#include <ntstatus.h>
#include <tchar.h>
#include <Psapi.h>

//REGQUERYVALUEEXW fpRegQueryValueExW = NULL;
LOADLIBRARYEXW fpLoadLibraryExW = NULL;

//LRESULT WINAPI RegQueryValueExW_hook(HKEY hKey, LPCTSTR lpValueName, LPDWORD lpReserved, LPDWORD lpType, LPBYTE lpData, LPDWORD lpcbData) {
//    static NTQUERYKEY fpNtQueryKey = NULL;
//
//    DWORD cbData = *lpcbData;
//
//    LRESULT result = fpRegQueryValueExW(hKey, lpValueName, lpReserved, lpType, lpData, lpcbData);
//    if ( !result && lpData && !_wcsicmp((const wchar_t *)lpValueName, L"ServiceDll") ) {
//        if ( InitNTDLL() ) {
//            if ( !fpNtQueryKey )
//                fpNtQueryKey = (NTQUERYKEY)GetProcAddress(g_hNTDLL, "NtQueryKey");
//
//            if ( fpNtQueryKey ) {
//                ULONG ResultLength;
//                NTSTATUS status = fpNtQueryKey((HANDLE)hKey, KeyNameInformation, NULL, 0, &ResultLength);
//                if ( status == STATUS_BUFFER_OVERFLOW || status == STATUS_BUFFER_TOO_SMALL ) {
//                    PVOID buffer = malloc(ResultLength);
//                    if ( NT_SUCCESS(fpNtQueryKey((HANDLE)hKey, KeyNameInformation, buffer, ResultLength, &ResultLength))
//                        && !_wcsnicmp(buffer, L"\\REGISTRY\\MACHINE\\SYSTEM\\CurrentControlSet\\services\\wuauserv", (size_t)ResultLength) ) {
//
//                        wchar_t path[MAX_PATH], newpath[MAX_PATH];
//                        wcsncpy_s(path, _countof(path), (wchar_t *)lpData, (rsize_t)*lpcbData);
//
//                        if ( ApplyUpdatePack7R2ShimIfNeeded(path, _countof(path), newpath, _countof(newpath)) ) {
//                            trace(_T("UpdatePack7R2 shim: \"%ls\" -> \"%ls\""), path, newpath);
//                            wcscpy_s((wchar_t *)lpData, (rsize_t)cbData, newpath);
//                        }
//                    }
//                    free(buffer);
//                }
//            }
//        }
//    }
//    return result;
//}

HMODULE WINAPI LoadLibraryExW_hook(LPCWSTR lpFileName, HANDLE hFile, DWORD dwFlags) {
    wchar_t buffer[MAX_PATH];
    wcscpy_s(buffer, _countof(buffer), lpFileName);

    if ( !_wcsicmp(buffer, GetWindowsUpdateServiceDllW())
        && ApplyUpdatePack7R2ShimIfNeeded(buffer, _countof(buffer), buffer, _countof(buffer)) ) {
        trace(_T("UpdatePack7R2 shim: %ls -> %ls"), lpFileName, buffer);
    }

    return fpLoadLibraryExW((LPWSTR)buffer, hFile, dwFlags);
}
