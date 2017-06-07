#include <Windows.h>
#include <TlHelp32.h>
#include <tchar.h>
#include "service.h"
#include "process.h"
#include "util.h"

void CALLBACK Rundll32Entry(HWND hwnd, HINSTANCE hinst, LPSTR lpszCmdLine, int nCmdShow) {
    HANDLE hEvent = OpenEvent(SYNCHRONIZE, FALSE, _T("Global\\wufuc_UnloadEvent"));
    if (hEvent) {
        CloseHandle(hEvent);
        return;
    }
    SC_HANDLE hSCManager = OpenSCManager(NULL, SERVICES_ACTIVE_DATABASE, SC_MANAGER_CONNECT);
    TCHAR lpGroupName[256];
    DWORD dwProcessId;
    BOOL result = get_svcpid(hSCManager, _T("wuauserv"), &dwProcessId);
    if (!result && get_svcgname(hSCManager, _T("wuauserv"), lpGroupName, _countof(lpGroupName))) {
        result = get_svcgpid(hSCManager, lpGroupName, &dwProcessId);
    }
    CloseServiceHandle(hSCManager);
    if (!result) {
        return;
    }
    TCHAR lpLibFileName[MAX_PATH + 1];
    GetModuleFileName(HINST_THISCOMPONENT, lpLibFileName, _countof(lpLibFileName));
    InjectLibrary(dwProcessId, lpLibFileName, _countof(lpLibFileName));
}

void CALLBACK Rundll32Unload(HWND hwnd, HINSTANCE hinst, LPSTR lpszCmdLine, int nCmdShow) {
    HANDLE hEvent = OpenEvent(EVENT_MODIFY_STATE, FALSE, _T("Global\\wufuc_UnloadEvent"));
    if (hEvent) {
        _tdbgprintf(_T("Setting unload event..."));
        SetEvent(hEvent);
        CloseHandle(hEvent);
    }
}
