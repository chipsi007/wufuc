#pragma once

EXTERN_C IMAGE_DOS_HEADER __ImageBase;
#define HINST_THISCOMPONENT ((HINSTANCE)&__ImageBase)

extern BOOL g_IsWindows7;
extern BOOL g_IsWindows8Point1;

LPVOID *FindIAT(HMODULE hModule, LPSTR lpFuncName);
VOID DetourIAT(HMODULE hModule, LPSTR lpFuncName, LPVOID *lpOldAddress, LPVOID lpNewAddress);

VOID SuspendProcessThreads(DWORD dwProcessId, DWORD dwThreadId, HANDLE *lphThreads, SIZE_T dwSize, SIZE_T *lpcb);
VOID ResumeAndCloseThreads(HANDLE *lphThreads, SIZE_T dwSize);

BOOL CompareWindowsVersion(BYTE Operator, DWORD dwMajorVersion, DWORD dwMinorVersion, WORD wServicePackMajor, WORD wServicePackMinor, DWORD dwTypeMask);
BOOL IsOperatingSystemSupported(LPBOOL lpbIsWindows7, LPBOOL lpbIsWindows8Point1);

VOID dwprintf_(LPCWSTR format, ...);

#define DETOUR_IAT(x, y) \
    LPVOID _LPORIGINAL##y; \
    DetourIAT(x, #y, &_LPORIGINAL##y, &_##y)
#define RESTORE_IAT(x, y) \
    DetourIAT(x, #y, NULL, _LPORIGINAL##y)

BOOL init_log(void);
VOID close_log(void);

#define STRINGIZE_(x) #x
#define STRINGIZE(x) STRINGIZE_(x)

#define STRINGIZEW_(x) L#x
#define STRINGIZEW(x) STRINGIZEW_(x)
#define __LINEWSTR__ STRINGIZEW(__LINE__)
#define dwprintf(format, ...) dwprintf_(__FILEW__ L"(" __LINEWSTR__ L"): " format L"\n", ##__VA_ARGS__)
