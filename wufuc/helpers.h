#pragma once

EXTERN_C IMAGE_DOS_HEADER __ImageBase;
#define HINST_THISCOMPONENT ((HINSTANCE)&__ImageBase)

typedef BOOL(WINAPI *ISWOW64PROCESS)(HANDLE, PBOOL);

BOOL CompareWindowsVersion(BYTE Operator, DWORD dwMajorVersion, DWORD dwMinorVersion, WORD wServicePackMajor, WORD wServicePackMinor, DWORD dwTypeMask);
BOOL IsWindows7(void);
BOOL IsWindows8Point1(void);
BOOL IsOperatingSystemSupported(void);
BOOL IsWow64(void);

VOID suspend_other_threads(DWORD dwProcessId, DWORD dwThreadId, HANDLE *lphThreads, SIZE_T dwSize, SIZE_T *lpcb);
VOID resume_and_close_threads(LPHANDLE lphThreads, SIZE_T dwSize);

void get_cpuid_brand(char *brand);
#define STRINGIZE_(x) #x
#define STRINGIZE(x) STRINGIZE_(x)
