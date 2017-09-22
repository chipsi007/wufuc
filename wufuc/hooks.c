#include "hooks.h"

#include "helpers.h"
#include "patchwua.h"
#include "ntdllhelper.h"
#include "tracing.h"

#include <stdint.h>

#define WIN32_NO_STATUS
#include <Windows.h>
#undef WIN32_NO_STATUS

#include <ntstatus.h>
#include <tchar.h>
#include <Psapi.h>

REGQUERYVALUEEXW pfnRegQueryValueExW = NULL;
LOADLIBRARYEXW pfnLoadLibraryExW = NULL;

LRESULT WINAPI RegQueryValueExW_hook(HKEY hKey, LPCWSTR lpValueName, LPDWORD lpReserved, LPDWORD lpType, LPBYTE lpData, LPDWORD lpcbData) {
    size_t nMaxCount = *lpcbData / sizeof(wchar_t);

    LRESULT result = pfnRegQueryValueExW(hKey, lpValueName, lpReserved, lpType, lpData, lpcbData);

    NTSTATUS status;
    ULONG ResultLength;
    if ( !_wcsicmp(lpValueName, L"ServiceDll")
        && TryNtQueryKey((HANDLE)hKey, KeyNameInformation, NULL, 0, &ResultLength, &status)
        && (status == STATUS_BUFFER_OVERFLOW || status == STATUS_BUFFER_TOO_SMALL) ) {

        PKEY_NAME_INFORMATION pkni = malloc(ResultLength);
        if ( TryNtQueryKey((HANDLE)hKey, KeyNameInformation, (PVOID)pkni, ResultLength, &ResultLength, &status)
            && NT_SUCCESS(status) ) {

            size_t nBufferCount = pkni->NameLength / sizeof(wchar_t);
            int current, pos;
            if ( _snwscanf_s(pkni->Name, nBufferCount, L"\\REGISTRY\\MACHINE\\SYSTEM\\ControlSet%03d\\services\\wuauserv\\Parameters%n", &current, &pos) == 1
                && pos == nBufferCount ) {

                size_t nCount = nMaxCount + 1;
                wchar_t *path = calloc(nCount, sizeof(wchar_t));
                wcsncpy_s(path, nCount, (wchar_t *)lpData, *lpcbData / sizeof(wchar_t));

                wchar_t *fname = find_fname(path);
                if ( !_wcsicmp(fname, L"wuaueng2.dll")      // UpdatePack7R2
                    || !_wcsicmp(fname, L"WuaCpuFix64.dll") // WuaCpuFix
                    || !_wcsicmp(fname, L"WuaCpuFix.dll") ) {

                    wcscpy_s(fname, nMaxCount - (fname - path), L"wuaueng.dll");

                    DWORD nSize = ExpandEnvironmentStringsW(path, NULL, 0);
                    wchar_t *lpDst = calloc(nSize, sizeof(wchar_t));
                    ExpandEnvironmentStringsW(path, lpDst, nSize);
                    if ( GetFileAttributesW(lpDst) != INVALID_FILE_ATTRIBUTES ) {
                        trace(_T("Compatibility fix: %.*ls -> %ls"), *lpcbData / sizeof(wchar_t), (wchar_t *)lpData, path);
                        size_t nLength = wcsnlen_s(path, nMaxCount);
                        wcsncpy_s((wchar_t *)lpData, nMaxCount, path, nLength);
                        *lpcbData = (DWORD)(nLength * sizeof(wchar_t));
                    }
                    free(lpDst);
                }
                free(path);
            }
        }
        free(pkni);
    }
    return result;
}

HMODULE WINAPI LoadLibraryExW_hook(LPCWSTR lpFileName, HANDLE hFile, DWORD dwFlags) {
    HMODULE result = pfnLoadLibraryExW(lpFileName, hFile, dwFlags);
    trace(_T("Loaded library: %ls"), lpFileName);

    DWORD dwLen = GetFileVersionInfoSizeW(lpFileName, NULL);
    if ( dwLen ) {
        LPVOID pBlock = malloc(dwLen);
        if ( GetFileVersionInfoW(lpFileName, 0, dwLen, pBlock) ) {
            PLANGANDCODEPAGE ptl;
            UINT cb;
            if ( VerQueryValueW(pBlock, L"\\VarFileInfo\\Translation", (LPVOID *)&ptl, &cb) ) {
                wchar_t lpSubBlock[38];
                for ( size_t i = 0; i < (cb / sizeof(LANGANDCODEPAGE)); i++ ) {
                    swprintf_s(lpSubBlock, _countof(lpSubBlock), L"\\StringFileInfo\\%04x%04x\\InternalName", ptl[i].wLanguage, ptl[i].wCodePage);
                    wchar_t *lpszInternalName;
                    UINT uLen;
                    if ( VerQueryValueW(pBlock, lpSubBlock, (LPVOID *)&lpszInternalName, &uLen)
                        && !_wcsicmp(lpszInternalName, L"wuaueng.dll") ) {

                        VS_FIXEDFILEINFO *pffi;
                        VerQueryValueW(pBlock, L"\\", (LPVOID *)&pffi, &uLen);
                        WORD wMajor = HIWORD(pffi->dwProductVersionMS);
                        WORD wMinor = LOWORD(pffi->dwProductVersionMS);
                        WORD wBuild = HIWORD(pffi->dwProductVersionLS);
                        WORD wRevision = LOWORD(pffi->dwProductVersionLS);

                        TCHAR fname[MAX_PATH];
                        GetModuleBaseName(GetCurrentProcess(), result, fname, _countof(fname));
                        
                        if ( (IsWindows7() && compare_versions(wMajor, wMinor, wBuild, wRevision, 7, 6, 7601, 23714) != -1)
                            || (IsWindows8Point1() && compare_versions(wMajor, wMinor, wBuild, wRevision, 7, 9, 9600, 18621) != -1) ) {

                            trace(_T("%s version: %d.%d.%d.%d"), fname, wMajor, wMinor, wBuild, wRevision);
                            MODULEINFO modinfo;
                            if ( GetModuleInformation(GetCurrentProcess(), result, &modinfo, sizeof(MODULEINFO)) ) {
                                if ( PatchWUA(modinfo.lpBaseOfDll, modinfo.SizeOfImage) )
                                    trace(_T("Successfully patched %s!"), fname);
                                else trace(_T("Failed to patch %s!"), fname);
                            }
                                
                            else trace(_T("Failed to get module information for %s (%p) (couldn't patch)"), fname, result);
                        } else trace(_T("Unsupported version of %s: %d.%d.%d.%d (patching skipped)"), fname, wMajor, wMinor, wBuild, wRevision);
                        break;
                    }
                }
            }
        }
        free(pBlock);
    }
    return result;
}
