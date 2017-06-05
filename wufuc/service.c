#include <windows.h>
#include <tchar.h>
#include "util.h"
#include "service.h"
#include "shellapihelper.h"

BOOL get_svcpath(SC_HANDLE hSCManager, LPCTSTR lpServiceName, LPTSTR lpBinaryPathName, SIZE_T dwSize) {
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

BOOL get_svcpid(SC_HANDLE hSCManager, LPCTSTR lpServiceName, DWORD *lpdwProcessId) {
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
	}
	CloseServiceHandle(hService);
	return result;
}

BOOL get_svcgname(SC_HANDLE hSCManager, LPCTSTR lpServiceName, LPTSTR lpGroupName, SIZE_T dwSize) {
	TCHAR lpBinaryPathName[0x8000];
	if (!get_svcpath(hSCManager, lpServiceName, lpBinaryPathName, _countof(lpBinaryPathName))) {
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
		for (int i = 1; i < numArgs; i++) {
			if (!_tcsicmp(*(p++), _T("-k"))) {
				_tcscpy_s(lpGroupName, dwSize, *p);
				result = TRUE;
				break;
			}
		}
	}
	return result;
}

BOOL get_svcgpid(SC_HANDLE hSCManager, LPTSTR lpServiceGroupName, DWORD *lpdwProcessId) {
	DWORD uBytes = 0x100000;
	LPBYTE pvData = malloc(uBytes);

	RegGetValue(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Svchost"), lpServiceGroupName, RRF_RT_REG_MULTI_SZ, NULL, pvData, &uBytes);

	BOOL result = FALSE;
	for (LPTSTR p = (LPTSTR)pvData; *p; p += _tcslen(p) + 1) {
		DWORD dwProcessId;
		TCHAR group[256];
		if (get_svcpid(hSCManager, p, &dwProcessId)) {
			get_svcgname(hSCManager, p, group, _countof(group));
			result = !_tcsicmp(group, lpServiceGroupName);
		}
		if (result) {
			*lpdwProcessId = dwProcessId;
			break;
		}
	}
	LocalFree(pvData);
	return result;
}
