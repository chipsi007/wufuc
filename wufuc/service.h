#pragma once


BOOL get_svcdllA(LPCSTR lpServiceName, LPSTR lpServiceDll, DWORD dwSize);

BOOL get_svcdllW(LPCWSTR lpServiceName, LPWSTR lpServiceDll, DWORD dwSize);

#ifdef UNICODE
#define get_svcdll get_svcdllW
#else
#define get_svcdll get_svcdllA
#endif

BOOL get_svcpid(SC_HANDLE hSCManager, LPCTSTR lpServiceName, DWORD *lpdwProcessId);

BOOL get_svcgname(SC_HANDLE hSCManager, LPCTSTR lpServiceName, LPTSTR lpGroupName, SIZE_T dwSize);

BOOL get_svcpath(SC_HANDLE hSCManager, LPCTSTR lpServiceName, LPTSTR lpBinaryPathName, SIZE_T dwSize);

BOOL get_svcgpid(SC_HANDLE hSCManager, LPTSTR lpServiceGroupName, DWORD *lpdwProcessId);
