#include <Windows.h>
#include <Psapi.h>
#include <TlHelp32.h>
#include <tchar.h>
#include <aclapi.h>
#include <sddl.h>
#include "core.h"
#include "process.h"
#include "service.h"
#include "util.h"

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
    sa.nLength = sizeof(sa);
    ConvertStringSecurityDescriptorToSecurityDescriptor(_T("D:PAI(A;;FA;;;BA)"), SDDL_REVISION_1, &(sa.lpSecurityDescriptor), NULL);
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

    HMODULE hwu = GetModuleHandle(_T("wuaueng.dll"));
    if (hwu) {
        PatchWUModule(hwu);
    }
    ResumeAndCloseThreads(lphThreads, cb);

    WaitForSingleObject(hEvent, INFINITE);

    _tdbgprintf(_T("Received wufuc_UnloadEvent, removing hooks."));

    SuspendProcessThreads(dwProcessId, dwThreadId, lphThreads, _countof(lphThreads), &cb);
    RESTORE_IAT(hm, LoadLibraryExA);
    RESTORE_IAT(hm, LoadLibraryExW);
    ResumeAndCloseThreads(lphThreads, cb);

    _tdbgprintf(_T("Unloading library. Cya!"));
    CloseHandle(hEvent);
    FreeLibraryAndExitThread(HINST_THISCOMPONENT, 0);
    return 0;
}

BOOL IsWUModule(HMODULE hModule) {
    TCHAR lpBaseName[MAX_PATH + 1];
    GetModuleBaseName(GetCurrentProcess(), hModule, lpBaseName, _countof(lpBaseName));
    return !_tcsicmp(lpBaseName, _T("wuaueng.dll"));
}

BOOL PatchWUModule(HMODULE hModule) {
    if (!IsWindows7Or8Point1()) {
        return FALSE;
    }

    LPSTR lpszPattern;
    SIZE_T n1, n2;
#ifdef _WIN64
    lpszPattern =
        "FFF3"                  // push rbx
        "4883EC??"              // sub rsp,??
        "33DB"                  // xor ebx,ebx
        "391D????????"          // cmp dword ptr ds:[???????????],ebx
        "7508"                  // jnz $+8
        "8B05????????";         // mov eax,dword ptr ds:[???????????]
    n1 = 10;
    n2 = 18;
#elif defined(_WIN32)
    if (IsWindows8Point1()) {
        lpszPattern =
            "8BFF"              // mov edi,edi
            "51"                // push ecx
            "833D????????00"    // cmp dword ptr ds:[????????],0
            "7507"              // jnz $+7
            "A1????????";       // mov eax,dword ptr ds:[????????]
        n1 = 5;
        n2 = 13;
    } else if (IsWindows7()) {
        lpszPattern =
            "833D????????00"    // cmp dword ptr ds:[????????],0
            "743E"              // je $+3E
            "E8????????"        // call <wuaueng.IsCPUSupported>
            "A3????????";       // mov dword ptr ds:[????????],eax
        n1 = 2;
        n2 = 15;
    }
#else
    return FALSE;
#endif

    MODULEINFO modinfo;
    GetModuleInformation(GetCurrentProcess(), hModule, &modinfo, sizeof(MODULEINFO));

    SIZE_T rva;
    if (!FindPattern(modinfo.lpBaseOfDll, modinfo.SizeOfImage, lpszPattern, 0, &rva)) {
        return FALSE;
    }
    SIZE_T fpIsDeviceServiceable = (SIZE_T)modinfo.lpBaseOfDll + rva;
    _tdbgprintf(_T("IsDeviceServiceable(void) matched at %p"), fpIsDeviceServiceable);

    BOOL *lpbNotRunOnce = (BOOL *)(fpIsDeviceServiceable + n1 + sizeof(DWORD) + *(DWORD *)(fpIsDeviceServiceable + n1));
    if (*lpbNotRunOnce) {
        *lpbNotRunOnce = FALSE;
        _tdbgprintf(_T("Patched %p=%d"), lpbNotRunOnce, *lpbNotRunOnce);
    }

    BOOL *lpbCachedResult = (BOOL *)(fpIsDeviceServiceable + n2 + sizeof(DWORD) + *(DWORD *)(fpIsDeviceServiceable + n2));
    if (!*lpbCachedResult) {
        *lpbCachedResult = TRUE;
        _tdbgprintf(_T("Patched %p=%d"), lpbCachedResult, *lpbCachedResult);
    }
    return TRUE;
}

HMODULE WINAPI _LoadLibraryExA(
    _In_       LPCSTR  lpFileName,
    _Reserved_ HANDLE  hFile,
    _In_       DWORD   dwFlags
) {
    HMODULE result = LoadLibraryExA(lpFileName, hFile, dwFlags);
    if (IsWUModule(result)) {
        PatchWUModule(result);
    }
    return result;
}

HMODULE WINAPI _LoadLibraryExW(
    _In_       LPCWSTR lpFileName,
    _Reserved_ HANDLE  hFile,
    _In_       DWORD   dwFlags
) {
    HMODULE result = LoadLibraryExW(lpFileName, hFile, dwFlags);
    if (IsWUModule(result)) {
        PatchWUModule(result);
    }
    return result;
};
