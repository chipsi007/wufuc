#include "helpers.h"

#include "tracing.h"

#include <stdint.h>

#include <Windows.h>
#include <tchar.h>
#include <TlHelp32.h>

static BOOL CompareWindowsVersion(BYTE Operator, DWORD dwMajorVersion, DWORD dwMinorVersion, WORD wServicePackMajor, WORD wServicePackMinor, DWORD dwTypeMask) {
    OSVERSIONINFOEX osvi = { 0 };
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
    static BOOL checked = FALSE, isWindows7 = FALSE;

    if ( !checked ) {
        isWindows7 = CompareWindowsVersion(VER_EQUAL, 6, 1, 0, 0, VER_MAJORVERSION | VER_MINORVERSION);
        checked = TRUE;
    }
    return isWindows7;
}

BOOL IsWindows8Point1(void) {
    static BOOL checked = FALSE, isWindows8Point1 = FALSE;

    if ( !checked ) {
        isWindows8Point1 = CompareWindowsVersion(VER_EQUAL, 6, 3, 0, 0, VER_MAJORVERSION | VER_MINORVERSION);
        checked = TRUE;
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
    static BOOL checked = FALSE, isWow64 = FALSE;
    static ISWOW64PROCESS pfnIsWow64Process = NULL;

    if ( !checked ) {
        if ( !pfnIsWow64Process )
            pfnIsWow64Process = (ISWOW64PROCESS)GetProcAddress(GetModuleHandle(_T("kernel32.dll")), "IsWow64Process");

        if ( pfnIsWow64Process && pfnIsWow64Process(GetCurrentProcess(), &isWow64) )
            checked = TRUE;
    }
    return isWow64;
}

size_t suspend_other_threads(LPHANDLE lphThreads, size_t nMaxCount) {
    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
    THREADENTRY32 te = { 0 };
    te.dwSize = sizeof(te);
    size_t count = 0;
    if ( Thread32First(hSnap, &te) ) {
        DWORD dwProcessId = GetCurrentProcessId();
        DWORD dwThreadId = GetCurrentThreadId();
        WaitForTracingMutex(); // make sure we don't dead lock
        do {
            if ( te.th32OwnerProcessID != dwProcessId || te.th32ThreadID == dwThreadId )
                continue;

            lphThreads[count] = OpenThread(THREAD_SUSPEND_RESUME, FALSE, te.th32ThreadID);
            SuspendThread(lphThreads[count]);
            count++;
        } while ( count < nMaxCount && Thread32Next(hSnap, &te) );
        ReleaseTracingMutex();
    }
    CloseHandle(hSnap);
    trace(_T("Suspended %zu other threads"), count);
    return count;
}

void resume_and_close_threads(LPHANDLE lphThreads, size_t nCount) {
    for ( size_t i = 0; i < nCount; i++ ) {
        ResumeThread(lphThreads[i]);
        CloseHandle(lphThreads[i]);
    }
    trace(_T("Resumed %zu threads"), nCount);
}

char *get_cpuid_brand(char *brand, size_t nMaxCount) {
    int info[4];
    __cpuidex(info, 0x80000000, 0);
    if ( info[0] < 0x80000004 ) {
        brand[0] = '\0';
        return brand;
    }
    uint32_t *char_as_int = (uint32_t *)brand;
    for ( int op = 0x80000002; op <= 0x80000004; op++ ) {
        __cpuidex(info, op, 0);
        for ( int i = 0; i < 4; i++ )
            *(char_as_int++) = info[i];
    }
    size_t i = 0;
    while ( i < nMaxCount && isspace(brand[i]) )
        i++;

    return brand + i;
}

wchar_t *find_fname(wchar_t *pPath) {
    wchar_t *pwc = wcsrchr(pPath, L'\\');
    if ( pwc && *(++pwc) )
        return pwc;

    return pPath;
}

int compare_versions(WORD ma1, WORD mi1, WORD b1, WORD r1, WORD ma2, WORD mi2, WORD b2, WORD r2) {
    if ( ma1 < ma2 ) return -1;
    if ( ma1 > ma2 ) return 1;
    if ( mi1 < mi2 ) return -1;
    if ( mi1 > mi2 ) return 1;
    if ( b1 < b2 ) return -1;
    if ( b1 > b2 ) return 1;
    if ( r1 < r2 ) return -1;
    if ( r1 > r2 ) return 1;
    return 0;
}
