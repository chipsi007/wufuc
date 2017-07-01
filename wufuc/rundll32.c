#include <Windows.h>
#include <TlHelp32.h>
#include <tchar.h>
#include <VersionHelpers.h>
#include "service.h"
#include "util.h"

void CALLBACK Rundll32Entry(HWND hwnd, HINSTANCE hinst, LPSTR lpszCmdLine, int nCmdShow) {
    HANDLE hEvent = OpenEvent(SYNCHRONIZE, FALSE, _T("Global\\wufuc_UnloadEvent"));
    if (hEvent) {
        CloseHandle(hEvent);
        return;
    }

    LPWSTR osname;
    if (IsWindows7()) {
        if (IsWindowsServer()) {
            osname = L"Windows Server 2008 R2";
        } else {
            osname = L"Windows 7";
        }
    } else if (IsWindows8Point1()) {
        if (IsWindowsServer()) {
            osname = L"Windows Server 2012 R2";
        } else {
            osname = L"Windows 8.1";
        }
    }
    dwprintf(L"Operating System: %s %d-bit", osname, sizeof(uintptr_t) * 8);

    char brand[0x31];
    get_cpuid_brand(brand);
    SIZE_T i = 0;
    while (i < _countof(brand) && isspace(*(brand + i))) {
        i++;
    }
    dwprintf(L"Processor: %S", brand + i);

    SC_HANDLE hSCManager = OpenSCManager(NULL, SERVICES_ACTIVE_DATABASE, SC_MANAGER_CONNECT);
    if (!hSCManager) {
        return;
    }
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
    TCHAR lpLibFileName[MAX_PATH];
    GetModuleFileName(HINST_THISCOMPONENT, lpLibFileName, _countof(lpLibFileName));

    HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwProcessId);
    if (!hProcess) {
        return;
    }
    LPVOID lpBaseAddress = VirtualAllocEx(hProcess, NULL, sizeof(lpLibFileName), MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    if (lpBaseAddress && WriteProcessMemory(hProcess, lpBaseAddress, lpLibFileName, sizeof(lpLibFileName), NULL)) {

        HANDLE hThread = CreateRemoteThread(hProcess, NULL, 0, 
            (LPTHREAD_START_ROUTINE)GetProcAddress(GetModuleHandle(L"kernel32.dll"), 
            STRINGIZE(LoadLibrary)), 
            lpBaseAddress, 0, NULL
        );
        WaitForSingleObject(hThread, INFINITE);
        dwprintf(L"Injected into process: %d", dwProcessId);
        CloseHandle(hThread);
    }
    VirtualFreeEx(hProcess, lpBaseAddress, 0, MEM_RELEASE);
    CloseHandle(hProcess);
    close_log();
}

void CALLBACK Rundll32Unload(HWND hwnd, HINSTANCE hinst, LPSTR lpszCmdLine, int nCmdShow) {
    HANDLE hEvent = OpenEvent(EVENT_MODIFY_STATE, FALSE, _T("Global\\wufuc_UnloadEvent"));
    if (hEvent) {
        dwprintf(L"Setting unload event...");
        SetEvent(hEvent);
        CloseHandle(hEvent);
    }
}
