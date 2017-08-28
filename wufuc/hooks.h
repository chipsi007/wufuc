#ifndef HOOKS_H
#define HOOKS_H
#pragma once

//#define WIN32_NO_STATUS
#include <Windows.h>
//#undef WIN32_NO_STATUS

//typedef enum _KEY_INFORMATION_CLASS {
//    KeyBasicInformation = 0,
//    KeyNodeInformation = 1,
//    KeyFullInformation = 2,
//    KeyNameInformation = 3,
//    KeyCachedInformation = 4,
//    KeyFlagsInformation = 5,
//    KeyVirtualizationInformation = 6,
//    KeyHandleTagsInformation = 7,
//    MaxKeyInfoClass = 8
//} KEY_INFORMATION_CLASS;
//
//typedef NTSTATUS(NTAPI *NTQUERYKEY)(HANDLE, KEY_INFORMATION_CLASS, PVOID, ULONG, PULONG);
//
//typedef LRESULT(WINAPI *REGQUERYVALUEEXW)(HKEY, LPCWSTR, LPDWORD, LPDWORD, LPBYTE, LPDWORD);
typedef HMODULE(WINAPI *LOADLIBRARYEXW)(LPCWSTR, HANDLE, DWORD);

//extern REGQUERYVALUEEXW fpRegQueryValueExW;
extern LOADLIBRARYEXW fpLoadLibraryExW;

LRESULT WINAPI RegQueryValueExW_hook(HKEY hKey, LPCTSTR lpValueName, LPDWORD lpReserved, LPDWORD lpType, LPBYTE lpData, LPDWORD lpcbData);

HMODULE WINAPI LoadLibraryExW_hook(LPCWSTR lpFileName, HANDLE  hFile, DWORD dwFlags);

#endif
