#ifndef SERVICE_H
#define SERVICE_H
#pragma once

#include <Windows.h>

BOOL FindServiceDllW(LPCWSTR lpServiceName, LPWSTR lpServiceDll, DWORD nCount);
LPWSTR GetWindowsUpdateServiceDllW(void);
BOOL ApplyUpdatePack7R2ShimIfNeeded(const wchar_t *path, size_t pathsize, wchar_t *newpath, size_t newpathsize);
BOOL GetServiceProcessId(SC_HANDLE hSCManager, LPCTSTR lpServiceName, LPDWORD lpdwProcessId);
BOOL GetServiceGroupName(SC_HANDLE hSCManager, LPCTSTR lpServiceName, LPTSTR lpGroupName, SIZE_T dwSize);
BOOL GetServiceCommandLine(SC_HANDLE hSCManager, LPCTSTR lpServiceName, LPTSTR lpBinaryPathName, SIZE_T dwSize);
BOOL GetServiceGroupProcessId(SC_HANDLE hSCManager, LPTSTR lpServiceGroupName, LPDWORD lpdwProcessId);
#endif
