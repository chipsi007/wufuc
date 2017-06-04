#include <windows.h>
#include <tchar.h>
#include "util.h"
#include "service.h"

BOOL QueryServiceBinaryPathName(SC_HANDLE hSCManager, LPCTSTR lpServiceName, LPTSTR lpBinaryPathName, SIZE_T dwSize) {
	HANDLE hService = OpenService(hSCManager, lpServiceName, SERVICE_QUERY_CONFIG);
	if (!hService) {
		return FALSE;
	}

	DWORD cbBytesNeeded;
	QueryServiceConfig(hService, NULL, 0, &cbBytesNeeded);
	LPQUERY_SERVICE_CONFIG sc = malloc(cbBytesNeeded);
	BOOL result = QueryServiceConfig(hService, sc, cbBytesNeeded, &cbBytesNeeded);
	int a = GetLastError();
	CloseServiceHandle(hService);
	if (result) {
		_tcscpy_s(lpBinaryPathName, dwSize, sc->lpBinaryPathName);
	}
	LocalFree(sc);
	return result;
}

BOOL QueryServiceProcessId(SC_HANDLE hSCManager, LPCTSTR lpServiceName, DWORD *lpdwProcessId) {
	SC_HANDLE hService = OpenService(hSCManager, lpServiceName, SERVICE_QUERY_STATUS);
	if (!hService) {
		return FALSE;
	}

	SERVICE_STATUS_PROCESS lpBuffer;
	DWORD cbBytesNeeded;
	BOOL result = FALSE;
	if (QueryServiceStatusEx(hService, SC_STATUS_PROCESS_INFO, (LPBYTE)&lpBuffer, sizeof(lpBuffer), &cbBytesNeeded) && lpBuffer.dwProcessId) {
		*lpdwProcessId = lpBuffer.dwProcessId;
		result = TRUE;
		DbgPrint("Found %s pid %d", lpServiceName, *lpdwProcessId);
	}
	CloseServiceHandle(hService);
	return result;
}

BOOL GetServiceGroupName(SC_HANDLE hSCManager, LPCTSTR lpServiceName, LPTSTR lpGroupName, SIZE_T dwSize) {
	TCHAR lpBinaryPathName[0x8000];
	if (!QueryServiceBinaryPathName(hSCManager, lpServiceName, lpBinaryPathName, _countof(lpBinaryPathName))) {
		return FALSE;
	}
	int numArgs;
	LPWSTR *argv = CommandLineToArgv(lpBinaryPathName, &numArgs);
	if (numArgs < 3) {
		return FALSE;
	}

	TCHAR fname[_MAX_FNAME];
	_tsplitpath_s(argv[0], NULL, 0, NULL, 0, fname, _countof(fname), NULL, 0);

	BOOL result = FALSE;
	if (!_tcsicmp(fname, _T("svchost"))) {
		LPWSTR *p = argv;
		for (int i = 0; i + 1 < numArgs; i++) {
			if (!_tcsicmp(*(p++), _T("-k"))) {
				_tcscpy_s(lpGroupName, dwSize, *p);
				result = TRUE;
				DbgPrint("Found %s svc group: %s", lpServiceName, lpGroupName);
				break;
			}
		}
	}
	return result;
}

BOOL FindServiceGroupProcessId(SC_HANDLE hSCManager, LPTSTR lpServiceGroupName, DWORD *lpdwProcessId) {
	DWORD uBytes = 0x100000;
	LPBYTE pvData = malloc(uBytes);

	RegGetValue(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Svchost"), lpServiceGroupName, RRF_RT_REG_MULTI_SZ, NULL, pvData, &uBytes);

	LPTSTR ptr = (LPTSTR)pvData;

	BOOL result = FALSE;
	while (*ptr) {
		DWORD dwProcessId;
		if (QueryServiceProcessId(hSCManager, ptr, &dwProcessId)) {
			TCHAR group[256];
			GetServiceGroupName(hSCManager, ptr, group, _countof(group));
			result = !_tcsicmp(group, lpServiceGroupName);
		}
		if (result) {
			DbgPrint("Found %s pid %d", lpServiceGroupName, dwProcessId);
			*lpdwProcessId = dwProcessId;
			break;
		}
		ptr += _tcslen(ptr) + 1;
	}
	LocalFree(pvData);
	return result;
}
