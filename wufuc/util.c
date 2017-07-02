#include <Windows.h>
#include <stdio.h>
#include <stdint.h>
#include <intrin.h> 
#include <tchar.h>
#include <TlHelp32.h>
#include <Psapi.h>
#include "util.h"

static BOOL checkedIsWindows7 = FALSE;
static BOOL isWindows7 = FALSE;
static BOOL checkedIsWindows8Point1 = FALSE;
static BOOL isWindows8Point1 = FALSE;

static LPFN_ISWOW64PROCESS fnIsWow64Process = NULL;
static BOOL checkedIsWow64 = FALSE;
static BOOL isWow64 = FALSE;

static FILE *log_fp = NULL;

LPVOID *FindIAT(HMODULE hModule, LPSTR lpFunctionName) {
    uintptr_t hm = (uintptr_t)hModule;

    for (PIMAGE_IMPORT_DESCRIPTOR iid = (PIMAGE_IMPORT_DESCRIPTOR)(hm + ((PIMAGE_NT_HEADERS)(hm + ((PIMAGE_DOS_HEADER)hm)->e_lfanew))
        ->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress); iid->Name; iid++) {

        LPVOID *p;
        for (SIZE_T i = 0; *(p = i + (LPVOID *)(hm + iid->FirstThunk)); i++) {
            LPSTR fn = (LPSTR)(hm + *(i + (SIZE_T *)(hm + iid->OriginalFirstThunk)) + 2);
            if (!((uintptr_t)fn & IMAGE_ORDINAL_FLAG) && !_stricmp(lpFunctionName, fn)) {
                return p;
            }
        }
    }
    return NULL;
}

VOID DetourIAT(HMODULE hModule, LPSTR lpFuncName, LPVOID *lpOldAddress, LPVOID lpNewAddress) {
    LPVOID *lpAddress = FindIAT(hModule, lpFuncName);
    if (!lpAddress || *lpAddress == lpNewAddress) {
        return;
    }

    DWORD flOldProtect;
    DWORD flNewProtect = PAGE_READWRITE;
    VirtualProtect(lpAddress, sizeof(LPVOID), flNewProtect, &flOldProtect);
    if (lpOldAddress) {
        *lpOldAddress = *lpAddress;
    }
    dwprintf(L"Modified %S import address: %p => %p", lpFuncName, *lpAddress, lpNewAddress);
    *lpAddress = lpNewAddress;
    VirtualProtect(lpAddress, sizeof(LPVOID), flOldProtect, &flNewProtect);
}

VOID SuspendProcessThreads(DWORD dwProcessId, DWORD dwThreadId, HANDLE *lphThreads, SIZE_T dwSize, SIZE_T *lpcb) {
    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
    THREADENTRY32 te;
    ZeroMemory(&te, sizeof(THREADENTRY32));
    te.dwSize = sizeof(te);
    Thread32First(hSnap, &te);

    SIZE_T count = 0;

    do {
        if (te.th32OwnerProcessID != dwProcessId || te.th32ThreadID == dwThreadId) {
            continue;
        }
        lphThreads[count] = OpenThread(THREAD_SUSPEND_RESUME, FALSE, te.th32ThreadID);
        SuspendThread(lphThreads[count]);
        count++;
    } while (count < dwSize && Thread32Next(hSnap, &te));
    CloseHandle(hSnap);

    *lpcb = count;
    dwprintf(L"Suspended %d other threads", count);
}

VOID ResumeAndCloseThreads(HANDLE *lphThreads, SIZE_T cb) {
    for (SIZE_T i = 0; i < cb; i++) {
        ResumeThread(lphThreads[i]);
        CloseHandle(lphThreads[i]);
    }
    dwprintf(L"Resumed %d other threads", cb);
}

BOOL CompareWindowsVersion(BYTE Operator, DWORD dwMajorVersion, DWORD dwMinorVersion, WORD wServicePackMajor, WORD wServicePackMinor, DWORD dwTypeMask) {
    OSVERSIONINFOEX osvi;
    ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));
    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
    osvi.dwMajorVersion = dwMajorVersion;
    osvi.dwMinorVersion = dwMinorVersion;
    osvi.wServicePackMajor = wServicePackMajor;
    osvi.wServicePackMinor = wServicePackMinor;

    DWORDLONG dwlConditionMask = 0;
    VER_SET_CONDITION(dwlConditionMask, VER_MAJORVERSION, Operator);
    VER_SET_CONDITION(dwlConditionMask, VER_MINORVERSION, Operator);
    VER_SET_CONDITION(dwlConditionMask, VER_SERVICEPACKMAJOR, Operator);
    VER_SET_CONDITION(dwlConditionMask, VER_SERVICEPACKMINOR, Operator);

    return VerifyVersionInfo(&osvi, dwTypeMask, dwlConditionMask);
}

BOOL IsWindows7(void) {
    if (!checkedIsWindows7) {
        isWindows7 = CompareWindowsVersion(VER_EQUAL, 6, 1, 0, 0, VER_MAJORVERSION | VER_MINORVERSION);
        checkedIsWindows7 = TRUE;
    }
    return isWindows7;
}

BOOL IsWindows8Point1(void) {
    if (!checkedIsWindows8Point1) {
        isWindows8Point1 = CompareWindowsVersion(VER_EQUAL, 6, 3, 0, 0, VER_MAJORVERSION | VER_MINORVERSION);
        checkedIsWindows8Point1 = TRUE;
    }
    return isWindows8Point1;
}

BOOL IsOperatingSystemSupported(void) {
#if !defined(_AMD64_) && !defined(_X86_)
    return FALSE;
#else
    return IsWindows7() || IsWindows8Point1();
#endif
}

BOOL IsWow64(void) {
    if (!checkedIsWow64) {
        if (!fnIsWow64Process) {
            fnIsWow64Process = (LPFN_ISWOW64PROCESS)GetProcAddress(GetModuleHandle(_T("kernel32.dll")), "IsWow64Process");
        }
        if (fnIsWow64Process && fnIsWow64Process(GetCurrentProcess(), &isWow64)) {
            checkedIsWow64 = TRUE;
        }
    }
    return isWow64;
}

void get_cpuid_brand(char* brand) {
    int info[4];
    __cpuidex(info, 0x80000000, 0);
    if (info[0] < 0x80000004) {
        brand[0] = '\0';
        return;
    }
    uint32_t *char_as_int = (uint32_t *)brand;
    for (int op = 0x80000002; op <= 0x80000004; op++) {
        __cpuidex(info, op, 0);
        *(char_as_int++) = info[0];
        *(char_as_int++) = info[1];
        *(char_as_int++) = info[2];
        *(char_as_int++) = info[3];
    }
}

BOOL init_log(void) {
    if (log_fp) {
        return TRUE;
    }
    WCHAR filename[MAX_PATH];
    GetModuleFileNameW(HINST_THISCOMPONENT, filename, _countof(filename));
    WCHAR drive[_MAX_DRIVE], dir[_MAX_DIR], fname[_MAX_FNAME];
    _wsplitpath_s(filename, drive, _countof(drive), dir, _countof(dir), fname, _countof(fname), NULL, 0);

    WCHAR basename[MAX_PATH];
    GetModuleBaseNameW(GetCurrentProcess(), NULL, basename, _countof(basename));
    wcscat_s(fname, _countof(fname), L".");
    wcscat_s(fname, _countof(fname), basename);
    _wmakepath_s(filename, _countof(filename), drive, dir, fname, L".log");

    HANDLE hFile = CreateFileW(filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    LARGE_INTEGER size;
    GetFileSizeEx(hFile, &size);
    CloseHandle(hFile);
    log_fp = _wfsopen(filename, size.QuadPart < (1 << 20) ? L"at" : L"wt", _SH_DENYWR);
    if (!log_fp) {
        return FALSE;
    }
    return TRUE;
}

VOID close_log(void) {
    if (log_fp) {
        fclose(log_fp);
    }
}

VOID dwprintf_(LPCWSTR format, ...) {
    if (init_log()) {
        WCHAR datebuf[9], timebuf[9];
        _wstrdate_s(datebuf, _countof(datebuf));
        _wstrtime_s(timebuf, _countof(timebuf));
        fwprintf_s(log_fp, L"%s %s [%d] ", datebuf, timebuf, GetCurrentProcessId());

        va_list argptr;
        va_start(argptr, format);
        vfwprintf_s(log_fp, format, argptr);
        va_end(argptr);
        fflush(log_fp);
    }
}
