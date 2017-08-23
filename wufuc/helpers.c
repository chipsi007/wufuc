#include <Windows.h>
#include <stdint.h>
#include <tchar.h>
#include <TlHelp32.h>
#include <Psapi.h>

#include "logging.h"
#include "helpers.h"

static BOOL m_checkedIsWindows7 = FALSE;
static BOOL m_isWindows7 = FALSE;
static BOOL m_checkedIsWindows8Point1 = FALSE;
static BOOL m_isWindows8Point1 = FALSE;

static ISWOW64PROCESS fpIsWow64Process = NULL;
static BOOL m_checkedIsWow64 = FALSE;
static BOOL m_isWow64 = FALSE;

static TCHAR m_emod_basename[MAX_PATH];

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
    if (!m_checkedIsWindows7) {
        m_isWindows7 = CompareWindowsVersion(VER_EQUAL, 6, 1, 0, 0, VER_MAJORVERSION | VER_MINORVERSION);
        m_checkedIsWindows7 = TRUE;
    }
    return m_isWindows7;
}

BOOL IsWindows8Point1(void) {
    if (!m_checkedIsWindows8Point1) {
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
    if (!m_checkedIsWow64) {
        if (!fpIsWow64Process)
            fpIsWow64Process = (ISWOW64PROCESS)GetProcAddress(GetModuleHandle(_T("kernel32.dll")), "IsWow64Process");
        
        if (fpIsWow64Process && fpIsWow64Process(GetCurrentProcess(), &m_isWow64))
            m_checkedIsWow64 = TRUE;
    }
    return m_isWow64;
}

VOID suspend_other_threads(DWORD dwProcessId, DWORD dwThreadId, HANDLE *lphThreads, SIZE_T dwSize, SIZE_T *lpcb) {
    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
    THREADENTRY32 te;
    ZeroMemory(&te, sizeof(THREADENTRY32));
    te.dwSize = sizeof(te);
    Thread32First(hSnap, &te);

    SIZE_T count = 0;

    do {
        if (te.th32OwnerProcessID != dwProcessId || te.th32ThreadID == dwThreadId)
            continue;
        
        lphThreads[count] = OpenThread(THREAD_SUSPEND_RESUME, FALSE, te.th32ThreadID);
        SuspendThread(lphThreads[count]);
        count++;
    } while (count < dwSize && Thread32Next(hSnap, &te));
    CloseHandle(hSnap);

    *lpcb = count;
    trace(L"Suspended %d other threads", count);
}

VOID resume_and_close_threads(LPHANDLE lphThreads, SIZE_T cb) {
    for (SIZE_T i = 0; i < cb; i++) {
        ResumeThread(lphThreads[i]);
        CloseHandle(lphThreads[i]);
    }
    trace(L"Resumed %d threads", cb);
}

void get_cpuid_brand(char *brand) {
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
