#include "helpers.h"

#include "logging.h"

#include <stdint.h>

#include <Windows.h>
#include <tchar.h>
#include <Psapi.h>
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
    static BOOL m_checkedIsWindows7 = FALSE;
    static BOOL m_isWindows7 = FALSE;

    if ( !m_checkedIsWindows7 ) {
        m_isWindows7 = CompareWindowsVersion(VER_EQUAL, 6, 1, 0, 0, VER_MAJORVERSION | VER_MINORVERSION);
        m_checkedIsWindows7 = TRUE;
    }
    return m_isWindows7;
}

BOOL IsWindows8Point1(void) {
    static BOOL m_checkedIsWindows8Point1 = FALSE;
    static BOOL m_isWindows8Point1 = FALSE;

    if ( !m_checkedIsWindows8Point1 ) {
        m_isWindows8Point1 = CompareWindowsVersion(VER_EQUAL, 6, 3, 0, 0, VER_MAJORVERSION | VER_MINORVERSION);
        m_checkedIsWindows8Point1 = TRUE;
    }
    return m_isWindows8Point1;
}

BOOL IsOperatingSystemSupported(void) {
#if !defined(_AMD64_) && !defined(_X86_)
    return FALSE;
#else
    return IsWindows7() || IsWindows8Point1();
#endif
}

BOOL IsWow64(void) {
    static BOOL m_checkedIsWow64 = FALSE;
    static BOOL m_isWow64 = FALSE;
    static ISWOW64PROCESS fpIsWow64Process = NULL;

    if ( !m_checkedIsWow64 ) {
        if ( !fpIsWow64Process )
            fpIsWow64Process = (ISWOW64PROCESS)GetProcAddress(GetModuleHandle(_T("kernel32.dll")), "IsWow64Process");

        if ( fpIsWow64Process && fpIsWow64Process(GetCurrentProcess(), &m_isWow64) )
            m_checkedIsWow64 = TRUE;
    }
    return m_isWow64;
}

void suspend_other_threads(LPHANDLE lphThreads, size_t *lpcb) {
    DWORD dwProcessId = GetCurrentProcessId();
    DWORD dwThreadId = GetCurrentThreadId();

    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
    THREADENTRY32 te = { 0 };
    te.dwSize = sizeof(te);
    Thread32First(hSnap, &te);

    size_t count = 0;
    do {
        if ( te.th32OwnerProcessID != dwProcessId || te.th32ThreadID == dwThreadId )
            continue;

        lphThreads[count] = OpenThread(THREAD_SUSPEND_RESUME, FALSE, te.th32ThreadID);
        SuspendThread(lphThreads[count]);
        count++;
    } while ( count < *lpcb && Thread32Next(hSnap, &te) );
    CloseHandle(hSnap);
    *lpcb = count;
    trace(_T("Suspended %d other threads"), count);
}

void resume_and_close_threads(LPHANDLE lphThreads, size_t cb) {
    for ( size_t i = 0; i < cb; i++ ) {
        ResumeThread(lphThreads[i]);
        CloseHandle(lphThreads[i]);
    }
    trace(_T("Resumed %d threads"), cb);
}

void get_cpuid_brand(char *brand) {
    int info[4];
    __cpuidex(info, 0x80000000, 0);
    if ( info[0] < 0x80000004 ) {
        brand[0] = '\0';
        return;
    }
    uint32_t *char_as_int = (uint32_t *)brand;
    for ( int op = 0x80000002; op <= 0x80000004; op++ ) {
        __cpuidex(info, op, 0);
        *(char_as_int++) = info[0];
        *(char_as_int++) = info[1];
        *(char_as_int++) = info[2];
        *(char_as_int++) = info[3];
    }
}
