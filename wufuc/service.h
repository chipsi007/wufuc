#pragma once

BOOL get_svcpath(SC_HANDLE hSCManager, LPCTSTR lpServiceName, LPTSTR lpBinaryPathName, SIZE_T dwSize);

BOOL get_svcpid(SC_HANDLE hSCManager, LPCTSTR lpServiceName, DWORD *lpdwProcessId);

BOOL get_svcgname(SC_HANDLE hSCManager, LPCTSTR lpServiceName, LPTSTR lpGroupName, SIZE_T dwSize);

BOOL get_svcgpid(SC_HANDLE hSCManager, LPTSTR lpServiceGroupName, DWORD *lpdwProcessId);
