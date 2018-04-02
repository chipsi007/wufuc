#pragma once


typedef LSTATUS(WINAPI *LPFN_REGQUERYVALUEEXW)(HKEY, LPCWSTR, LPDWORD, LPDWORD, LPBYTE, LPDWORD);
typedef HMODULE(WINAPI *LPFN_LOADLIBRARYEXW)(LPCWSTR, HANDLE, DWORD);

extern wchar_t *g_pszWUServiceDll;

extern LPFN_REGQUERYVALUEEXW g_pfnRegQueryValueExW;
extern LPFN_LOADLIBRARYEXW g_pfnLoadLibraryExW;

extern PVOID g_ptRegQueryValueExW;
extern PVOID g_ptLoadLibraryExW;

LSTATUS WINAPI RegQueryValueExW_hook(HKEY hKey, LPCWSTR lpValueName, LPDWORD lpReserved, LPDWORD lpType, LPBYTE lpData, LPDWORD lpcbData);
HMODULE WINAPI LoadLibraryExW_hook(LPCWSTR lpFileName, HANDLE hFile, DWORD dwFlags);
