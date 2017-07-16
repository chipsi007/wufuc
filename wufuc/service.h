#pragma once

BOOL get_svcdllA(LPCSTR lpServiceName, LPSTR lpServiceDll, DWORD dwSize);
BOOL get_svcdllW(LPCWSTR lpServiceName, LPWSTR lpServiceDll, DWORD dwSize);
BOOL get_svcpid(SC_HANDLE hSCManager, LPCTSTR lpServiceName, DWORD *lpdwProcessId);
BOOL get_svcgname(SC_HANDLE hSCManager, LPCTSTR lpServiceName, LPTSTR lpGroupName, SIZE_T dwSize);
BOOL get_svcpath(SC_HANDLE hSCManager, LPCTSTR lpServiceName, LPTSTR lpBinaryPathName, SIZE_T dwSize);
BOOL get_svcgpid(SC_HANDLE hSCManager, LPTSTR lpServiceGroupName, DWORD *lpdwProcessId);
LPSTR get_wuauservdllA(void);
LPWSTR get_wuauservdllW(void);

#ifdef UNICODE
#define get_svcdll get_svcdllW
#define get_wuauservdll get_wuauservdllW
#else
#define get_svcdll get_svcdllA
#define get_wuauservdll get_wuauservdllA
#endif
