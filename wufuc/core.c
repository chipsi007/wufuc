#include <Windows.h>
#include <stdint.h>
#include <tchar.h>
#include <Psapi.h>
#include <sddl.h>
#include "service.h"
#include "patternfind.h"
#include "util.h"
#include "core.h"

DWORD WINAPI NewThreadProc(LPVOID lpParam) {
    SC_HANDLE hSCManager = OpenSCManager(NULL, SERVICES_ACTIVE_DATABASE, SC_MANAGER_CONNECT);

    TCHAR lpBinaryPathName[0x8000];
    get_svcpath(hSCManager, _T("wuauserv"), lpBinaryPathName, _countof(lpBinaryPathName));

    BOOL result = _tcsicmp(GetCommandLine(), lpBinaryPathName);
    CloseServiceHandle(hSCManager);

    if (result) {
        return 0;
    }

    SECURITY_ATTRIBUTES sa;
    sa.nLength = sizeof(SECURITY_ATTRIBUTES);
    ConvertStringSecurityDescriptorToSecurityDescriptor(_T("D:PAI(A;;FA;;;BA)"), SDDL_REVISION_1, &sa.lpSecurityDescriptor, NULL);
    sa.bInheritHandle = FALSE;

    HANDLE hEvent = CreateEvent(&sa, TRUE, FALSE, _T("Global\\wufuc_UnloadEvent"));

    if (!hEvent) {
        return 0;
    }

    DWORD dwProcessId = GetCurrentProcessId();
    DWORD dwThreadId = GetCurrentThreadId();
    HANDLE lphThreads[0x1000];
    SIZE_T cb;

    SuspendProcessThreads(dwProcessId, dwThreadId, lphThreads, _countof(lphThreads), &cb);

    HMODULE hm = GetModuleHandle(NULL);
    DETOUR_IAT(hm, LoadLibraryExA);
    DETOUR_IAT(hm, LoadLibraryExW);

    TCHAR lpServiceDll[MAX_PATH + 1];
    get_svcdll(_T("wuauserv"), lpServiceDll, _countof(lpServiceDll));

    HMODULE hwu = GetModuleHandle(lpServiceDll);
    if (hwu && PatchWUAgentHMODULE(hwu)) {
        dwprintf(L"Patched previously loaded Windows Update module!");
    }
    ResumeAndCloseThreads(lphThreads, cb);

    WaitForSingleObject(hEvent, INFINITE);

    dwprintf(L"Unloading...");

    SuspendProcessThreads(dwProcessId, dwThreadId, lphThreads, _countof(lphThreads), &cb);
    RESTORE_IAT(hm, LoadLibraryExA);
    RESTORE_IAT(hm, LoadLibraryExW);
    ResumeAndCloseThreads(lphThreads, cb);

    CloseHandle(hEvent);
    dwprintf(L"Bye bye!");
    close_log();
    FreeLibraryAndExitThread(HINST_THISCOMPONENT, 0);
}

BOOL PatchWUAgentHMODULE(HMODULE hModule) {
    LPSTR pattern;
    SIZE_T offset00, offset01;
#ifdef _AMD64_
        pattern = "FFF3 4883EC?? 33DB 391D???????? 7508 8B05????????";
        offset00 = 10;
        offset01 = 18;
#elif defined(_X86_)
    if (IsWindows7()) {
        pattern = "833D????????00 743E E8???????? A3????????";
        offset00 = 2;
        offset01 = 15;
    } else if (IsWindows8Point1()) {
        pattern = "8BFF 51 833D????????00 7507 A1????????";
        offset00 = 5;
        offset01 = 13;
    } else {
        return FALSE;
    }
#endif

    MODULEINFO modinfo;
    GetModuleInformation(GetCurrentProcess(), hModule, &modinfo, sizeof(MODULEINFO));

    SIZE_T rva = patternfind(modinfo.lpBaseOfDll, modinfo.SizeOfImage, 0, pattern);
    if (rva == -1) {
        dwprintf(L"No pattern match!");
        return FALSE;
    }
    uintptr_t baseAddress = (uintptr_t)modinfo.lpBaseOfDll;
    uintptr_t lpfnIsDeviceServiceable = baseAddress + rva;
    dwprintf(L"Address of wuaueng.dll!IsDeviceServiceable: %p", lpfnIsDeviceServiceable);
    BOOL result = FALSE;
    LPBOOL lpbFirstRun, lpbIsCPUSupportedResult;
#ifdef _AMD64_
        lpbFirstRun = (LPBOOL)(lpfnIsDeviceServiceable + offset00 + sizeof(uint32_t) + *(uint32_t *)(lpfnIsDeviceServiceable + offset00));
        lpbIsCPUSupportedResult = (LPBOOL)(lpfnIsDeviceServiceable + offset01 + sizeof(uint32_t) + *(uint32_t *)(lpfnIsDeviceServiceable + offset01));
#elif defined(_X86_)
        lpbFirstRun = (LPBOOL)(*(uintptr_t *)(fpIsDeviceServiceable + offset00));
        lpbIsCPUSupportedResult = (LPBOOL)(*(uintptr_t *)(fpIsDeviceServiceable + offset01));
#endif

    if (*lpbFirstRun) {
        *lpbFirstRun = FALSE;
        dwprintf(L"Patched FirstRun variable: %p = %08x", lpbFirstRun, *lpbFirstRun);
        result = TRUE;
    }
    if (!*lpbIsCPUSupportedResult) {
        *lpbIsCPUSupportedResult = TRUE;
        dwprintf(L"Patched cached wuaueng.dll!IsCPUSupported result: %p = %08x", lpbIsCPUSupportedResult, *lpbIsCPUSupportedResult);
        result = TRUE;
    }
    return result;
}

HMODULE WINAPI _LoadLibraryExA(
    _In_       LPCSTR  lpFileName,
    _Reserved_ HANDLE  hFile,
    _In_       DWORD   dwFlags
) {
    HMODULE result = LoadLibraryExA(lpFileName, hFile, dwFlags);
    if (result) {
        dwprintf(L"Loaded library: %S", lpFileName);
        CHAR path[MAX_PATH + 1];
        if (!get_svcdllA("wuauserv", path, _countof(path))) {
            return result;
        }
        if (!_stricmp(lpFileName, path) && PatchWUAgentHMODULE(result)) {
            dwprintf(L"Patched Windows Update module!");
        }
    }
    return result;
}

HMODULE WINAPI _LoadLibraryExW(
    _In_       LPCWSTR lpFileName,
    _Reserved_ HANDLE  hFile,
    _In_       DWORD   dwFlags
) {
    HMODULE result = LoadLibraryExW(lpFileName, hFile, dwFlags);
    if (result) {
        dwprintf(L"Loaded library: %s", lpFileName);
        WCHAR path[MAX_PATH + 1];
        if (!get_svcdllW(L"wuauserv", path, _countof(path))) {
            return result;
        }
        if (!_wcsicmp(lpFileName, path) && PatchWUAgentHMODULE(result)) {
            dwprintf(L"Patched Windows Update module!");
        }
    }
    return result;
};
