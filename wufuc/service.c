#include "service.h"

#include <Windows.h>
#include <tchar.h>

BOOL GetServiceCommandLine(SC_HANDLE hSCManager, LPCTSTR lpServiceName, LPTSTR lpCommandLine, SIZE_T dwSize) {
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
            && !_tcscpy_s(lpCommandLine, dwSize, sc->lpBinaryPathName) )
            result = TRUE;
        free(sc);
    }
    CloseServiceHandle(hService);
    return result;
}
