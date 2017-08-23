#include <Windows.h>
#include <stdint.h>
#include <tchar.h>
#include <Psapi.h>
#include <sddl.h>

#include "helpers.h"
#include "logging.h"
#include "service.h"
#include "iathook.h"
#include "patternfind.h"
#include "hooks.h"

LOADLIBRARYEXW fpLoadLibraryExW = NULL;
LOADLIBRARYEXA fpLoadLibraryExA = NULL;

DWORD WINAPI NewThreadProc(LPVOID lpParam) {
    SC_HANDLE hSCManager = OpenSCManager(NULL, SERVICES_ACTIVE_DATABASE, SC_MANAGER_CONNECT);

    TCHAR lpBinaryPathName[0x8000];
    get_svcpath(hSCManager, _T("wuauserv"), lpBinaryPathName, _countof(lpBinaryPathName));
    CloseServiceHandle(hSCManager);

    if (_tcsicmp(GetCommandLine(), lpBinaryPathName))
        return 0;

    SECURITY_ATTRIBUTES sa = { 0 };
    sa.nLength = sizeof(SECURITY_ATTRIBUTES);
    ConvertStringSecurityDescriptorToSecurityDescriptor(_T("D:PAI(A;;FA;;;BA)"), SDDL_REVISION_1, &sa.lpSecurityDescriptor, NULL);
    sa.bInheritHandle = FALSE;

    HANDLE hEvent = CreateEvent(&sa, TRUE, FALSE, _T("Global\\wufuc_UnloadEvent"));
    if (!hEvent)
        return 0;

    DWORD dwProcessId = GetCurrentProcessId();
    DWORD dwThreadId = GetCurrentThreadId();
    HANDLE lphThreads[0x1000];
    SIZE_T count;

    suspend_other_threads(dwProcessId, dwThreadId, lphThreads, _countof(lphThreads), &count);

    HMODULE hm = GetModuleHandle(NULL);
    iat_hook(hm, "LoadLibraryExA", (LPVOID)&fpLoadLibraryExA, LoadLibraryExA_hook);
    iat_hook(hm, "LoadLibraryExW", (LPVOID)&fpLoadLibraryExW, LoadLibraryExW_hook);

    HMODULE hwu = GetModuleHandle(get_wuauservdll());
    if (hwu && PatchWUA(hwu))
        trace(L"Successfully patched previously loaded WUA module!");
    
    resume_and_close_threads(lphThreads, count);

    WaitForSingleObject(hEvent, INFINITE);
    trace(L"Unloading...");

    suspend_other_threads(dwProcessId, dwThreadId, lphThreads, _countof(lphThreads), &count);

    iat_hook(hm, "LoadLibraryExA", NULL, fpLoadLibraryExA);
    iat_hook(hm, "LoadLibraryExW", NULL, fpLoadLibraryExW);

    resume_and_close_threads(lphThreads, count);

    trace(L"Bye bye!");
    CloseHandle(hEvent);
    FreeLibraryAndExitThread(HINST_THISCOMPONENT, 0);
}

HMODULE WINAPI LoadLibraryExA_hook(
    _In_       LPCSTR  lpFileName,
    _Reserved_ HANDLE  hFile,
    _In_       DWORD   dwFlags
) {
    CHAR buffer[MAX_PATH];
    strcpy_s(buffer, _countof(buffer), lpFileName);

    BOOL isWUA = !_stricmp(buffer, get_wuauservdllA());
    if (isWUA) {
        CHAR drive[_MAX_DRIVE], dir[_MAX_DIR], fname[_MAX_FNAME], ext[_MAX_EXT];
        _splitpath_s(buffer, drive, _countof(drive), dir, _countof(dir), fname, _countof(fname), ext, _countof(ext));

        if (!_stricmp(fname, "wuaueng2")) {
            CHAR newpath[MAX_PATH];
            _makepath_s(newpath, _countof(buffer), drive, dir, "wuaueng", ext);

            if (GetFileAttributesA(newpath) != INVALID_FILE_ATTRIBUTES) {
                strcpy_s(buffer, _countof(buffer), newpath);
                trace(L"UpdatePack7R2 compatibility fix: redirecting %S -> %S", lpFileName, buffer);
            }
        }
    }

    HMODULE result = fpLoadLibraryExA(buffer, hFile, dwFlags);
    trace(L"Loaded library: %S", buffer);

    if (isWUA)
        PatchWUA(result);

    return result;
}

HMODULE WINAPI LoadLibraryExW_hook(
    _In_       LPCWSTR lpFileName,
    _Reserved_ HANDLE  hFile,
    _In_       DWORD   dwFlags
) {
    WCHAR buffer[MAX_PATH];
    wcscpy_s(buffer, _countof(buffer), lpFileName);

    BOOL isWUA = !_wcsicmp(buffer, get_wuauservdllW());
    if (isWUA) {
        WCHAR drive[_MAX_DRIVE], dir[_MAX_DIR], fname[_MAX_FNAME], ext[_MAX_EXT];
        _wsplitpath_s(buffer, drive, _countof(drive), dir, _countof(dir), fname, _countof(fname), ext, _countof(ext));

        if (!_wcsicmp(fname, L"wuaueng2")) {
            WCHAR newpath[MAX_PATH];
            _wmakepath_s(newpath, _countof(buffer), drive, dir, L"wuaueng", ext);

            if (GetFileAttributesW(newpath) != INVALID_FILE_ATTRIBUTES) {
                wcscpy_s(buffer, _countof(buffer), newpath);
                trace(L"UpdatePack7R2 compatibility fix: redirecting %s -> %s", lpFileName, buffer);
            }
        }
    }

    HMODULE result = fpLoadLibraryExW(buffer, hFile, dwFlags);
    trace(L"Loaded library: %s", buffer);

    if (isWUA)
        PatchWUA(result);

    return result;
}

BOOL PatchWUA(HMODULE hModule) {
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
    }
#endif

    MODULEINFO modinfo;
    GetModuleInformation(GetCurrentProcess(), hModule, &modinfo, sizeof(MODULEINFO));

    LPBYTE ptr = patternfind(modinfo.lpBaseOfDll, modinfo.SizeOfImage, 0, pattern);
    if (!ptr) {
        trace(L"No pattern match!");
        return FALSE;
    }
    trace(L"wuaueng!IsDeviceServiceable VA: %p", ptr);
    BOOL result = FALSE;
    LPBOOL lpbFirstRun, lpbIsCPUSupportedResult;
#ifdef _AMD64_
    lpbFirstRun = (LPBOOL)(ptr + offset00 + sizeof(uint32_t) + *(uint32_t *)(ptr + offset00));
    lpbIsCPUSupportedResult = (LPBOOL)(ptr + offset01 + sizeof(uint32_t) + *(uint32_t *)(ptr + offset01));
#elif defined(_X86_)
    lpbFirstRun = (LPBOOL)(*(uintptr_t *)(ptr + offset00));
    lpbIsCPUSupportedResult = (LPBOOL)(*(uintptr_t *)(ptr + offset01));
#endif

    if (*lpbFirstRun) {
        *lpbFirstRun = FALSE;
        trace(L"Patched boolean value #1: %p = %08x", lpbFirstRun, *lpbFirstRun);
        result = TRUE;
    }
    if (!*lpbIsCPUSupportedResult) {
        *lpbIsCPUSupportedResult = TRUE;
        trace(L"Patched boolean value #2: %p = %08x", lpbIsCPUSupportedResult, *lpbIsCPUSupportedResult);
        result = TRUE;
    }
    if (result)
        trace(L"Successfully patched WUA module!");

    return result;
}
