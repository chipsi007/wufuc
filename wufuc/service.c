#include "service.h"

#include <stdio.h>

#include <Windows.h>
#include <tchar.h>

#include "helpers.h"
#include "shellapihelper.h"
#include "logging.h"

static BOOL OpenServiceParametersKey(LPCWSTR lpSubKey, PHKEY phkResult) {
    BOOL result = FALSE;
    HKEY hKey, hSubKey;
    if ( !RegOpenKeyExW(HKEY_LOCAL_MACHINE, L"SYSTEM\\CurrentControlSet\\services", 0, KEY_READ, &hKey)
        && !RegOpenKeyExW(hKey, lpSubKey, 0, KEY_READ, &hSubKey)
        && !RegOpenKeyExW(hSubKey, L"Parameters", 0, KEY_READ, phkResult) ) {

        result = TRUE;
    }
    if ( hKey )
        RegCloseKey(hKey);

    if ( hSubKey )
        RegCloseKey(hSubKey);
    return result;
}

BOOL FindServiceDllW(LPCWSTR lpServiceName, LPWSTR lpServiceDll, DWORD dwSize) {
    BOOL result = FALSE;
    HKEY hKey;
    if ( OpenServiceParametersKey(lpServiceName, &hKey) ) {
        DWORD cb = dwSize;
        if ( !RegGetValueW(hKey, NULL, L"ServiceDll", RRF_RT_REG_SZ, NULL, lpServiceDll, &cb) ) {
            trace(_T("Service \"%s\" DLL path: %ls"), lpServiceName, lpServiceDll);
            result = TRUE;
        }
        RegCloseKey(hKey);
    }
    return result;
}

LPWSTR GetWindowsUpdateServiceDllW(void) {
    static WCHAR path[MAX_PATH];

    if ( !path[0] ) {
        if ( !FindServiceDllW(L"wuauserv", path, _countof(path)) )
            path[0] = L'\0';
    }

    return path;
}

BOOL ApplyUpdatePack7R2ShimIfNeeded(const wchar_t *path, size_t pathsize, wchar_t *newpath, size_t newpathsize) {
    wchar_t drive[_MAX_DRIVE], dir[_MAX_DIR], fname[_MAX_FNAME], ext[_MAX_EXT];
    _wsplitpath_s(path, drive, _countof(drive), dir, _countof(dir), fname, _countof(fname), ext, _countof(ext));
    if ( !_wcsicmp(fname, L"wuaueng2") ) {
        _wmakepath_s(newpath, newpathsize, drive, dir, L"wuaueng", ext);
        return GetFileAttributesW(newpath) != INVALID_FILE_ATTRIBUTES;
    }
    return FALSE;
}

BOOL GetServiceProcessId(SC_HANDLE hSCManager, LPCTSTR lpServiceName, LPDWORD lpdwProcessId) {
    BOOL result = FALSE;
    BOOL selfclose = !hSCManager;
    if ( selfclose && !(hSCManager = OpenSCManager(NULL, SERVICES_ACTIVE_DATABASE, SC_MANAGER_CONNECT)) )
        return result;

    SC_HANDLE hService = OpenService(hSCManager, lpServiceName, SERVICE_QUERY_STATUS);

    if ( selfclose )
        CloseServiceHandle(hSCManager);

    if ( !hService )
        return result;

    SERVICE_STATUS_PROCESS buffer;
    DWORD cb;
    if ( QueryServiceStatusEx(hService, SC_STATUS_PROCESS_INFO, (LPBYTE)&buffer, sizeof(buffer), &cb)
        && buffer.dwProcessId ) {

        *lpdwProcessId = buffer.dwProcessId;
        trace(_T("Service \"%s\" process ID: %d"), lpServiceName, *lpdwProcessId);
        result = TRUE;
    }
    CloseServiceHandle(hService);
    return result;
}

BOOL GetServiceGroupName(SC_HANDLE hSCManager, LPCTSTR lpServiceName, LPTSTR lpGroupName, SIZE_T dwSize) {
    BOOL result = FALSE;
    BOOL selfclose = !hSCManager;
    if ( selfclose && !(hSCManager = OpenSCManager(NULL, SERVICES_ACTIVE_DATABASE, SC_MANAGER_CONNECT)) )
        return result;

    TCHAR lpBinaryPathName[0x8000];
    if ( !GetServiceCommandLine(hSCManager, lpServiceName, lpBinaryPathName, _countof(lpBinaryPathName)) )
        return result;

    if ( selfclose )
        CloseServiceHandle(hSCManager);

    int numArgs;
    LPWSTR *argv = CommandLineToArgv(lpBinaryPathName, &numArgs);
    if ( numArgs < 3 )
        return result;

    TCHAR fname[_MAX_FNAME];
    _tsplitpath_s(argv[0], NULL, 0, NULL, 0, fname, _countof(fname), NULL, 0);

    if ( !_tcsicmp(fname, _T("svchost")) ) {
        LPWSTR *p = argv;
        for ( int i = 1; i < numArgs; i++ ) {
            if ( !_tcsicmp(*(p++), _T("-k")) && !_tcscpy_s(lpGroupName, dwSize, *p) ) {
                result = TRUE;
                trace(_T("Service \"%s\" group name: %s"), lpServiceName, lpGroupName);
                break;
            }
        }
    }
    return result;
}

BOOL GetServiceCommandLine(SC_HANDLE hSCManager, LPCTSTR lpServiceName, LPTSTR lpBinaryPathName, SIZE_T dwSize) {
    BOOL result = FALSE;
    if ( !hSCManager && !(hSCManager = OpenSCManager(NULL, SERVICES_ACTIVE_DATABASE, SC_MANAGER_CONNECT)) )
        return result;

    HANDLE hService = OpenService(hSCManager, lpServiceName, SERVICE_QUERY_CONFIG);
    if ( !hService )
        return result;

    DWORD cbBytesNeeded;
    if ( !QueryServiceConfig(hService, NULL, 0, &cbBytesNeeded)
        && GetLastError() == ERROR_INSUFFICIENT_BUFFER ) {

        LPQUERY_SERVICE_CONFIG sc = malloc(cbBytesNeeded);
        if ( QueryServiceConfig(hService, sc, cbBytesNeeded, &cbBytesNeeded)
            && !_tcscpy_s(lpBinaryPathName, dwSize, sc->lpBinaryPathName) )
            result = TRUE;
        free(sc);
    }
    CloseServiceHandle(hService);
    return result;
}

BOOL GetServiceGroupProcessId(SC_HANDLE hSCManager, LPTSTR lpServiceGroupName, LPDWORD lpdwProcessId) {
    BOOL result = FALSE;
    BOOL selfclose = !hSCManager;
    if ( selfclose && !(hSCManager = OpenSCManager(NULL, SERVICES_ACTIVE_DATABASE, SC_MANAGER_CONNECT)) )
        return result;

    DWORD uBytes = 1 << 20;
    LPBYTE pvData = malloc(uBytes);
    RegGetValue(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Svchost"),
        lpServiceGroupName, RRF_RT_REG_MULTI_SZ, NULL, pvData, &uBytes);

    for ( LPTSTR p = (LPTSTR)pvData; *p; p += _tcslen(p) + 1 ) {
        DWORD dwProcessId;
        TCHAR group[256];
        if ( GetServiceProcessId(hSCManager, p, &dwProcessId)
            && (GetServiceGroupName(hSCManager, p, group, _countof(group))
            && !_tcsicmp(group, lpServiceGroupName)) ) {

            *lpdwProcessId = dwProcessId;
            result = TRUE;
            trace(_T("Service group \"%s\" process ID: %d"), lpServiceGroupName, *lpdwProcessId);
            break;
        }
    }
    free(pvData);
    if ( selfclose )
        CloseServiceHandle(hSCManager);
    return result;
}
