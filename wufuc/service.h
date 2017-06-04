#pragma once

BOOL QueryServiceBinaryPathName(SC_HANDLE hSCManager, LPCTSTR lpServiceName, LPTSTR lpBinaryPathName, SIZE_T dwSize);

BOOL QueryServiceProcessId(SC_HANDLE hSCManager, LPCTSTR lpServiceName, DWORD *lpdwProcessId);

BOOL GetServiceGroupName(SC_HANDLE hSCManager, LPCTSTR lpServiceName, LPTSTR lpGroupName, SIZE_T dwSize);

BOOL FindServiceGroupProcessId(SC_HANDLE hSCManager, LPTSTR lpServiceGroupName, DWORD *lpdwProcessId);
