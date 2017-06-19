#pragma once

EXTERN_C IMAGE_DOS_HEADER __ImageBase;
#define HINST_THISCOMPONENT ((HINSTANCE)&__ImageBase)

LPVOID *FindIAT(HMODULE hModule, LPSTR lpFuncName);
VOID DetourIAT(HMODULE hModule, LPSTR lpFuncName, LPVOID *lpOldAddress, LPVOID lpNewAddress);

VOID SuspendProcessThreads(DWORD dwProcessId, DWORD dwThreadId, HANDLE *lphThreads, SIZE_T dwSize, SIZE_T *lpcb);
VOID ResumeAndCloseThreads(HANDLE *lphThreads, SIZE_T dwSize);

BOOL CompareWindowsVersion(BYTE Operator, DWORD dwMajorVersion, DWORD dwMinorVersion, WORD wServicePackMajor, WORD wServicePackMinor, DWORD dwTypeMask);
BOOL IsOperatingSystemSupported(LPBOOL lpbIsWindows7, LPBOOL lpbIsWindows8Point1);

VOID _wdbgprintf(LPCWSTR format, ...);
VOID _dbgprintf(LPCSTR format, ...);

#define DETOUR_IAT(x, y) \
    LPVOID _LPORIGINAL##y; \
    DetourIAT(x, #y, &_LPORIGINAL##y, &_##y)
#define RESTORE_IAT(x, y) \
    DetourIAT(x, #y, NULL, _LPORIGINAL##y)

#ifdef UNICODE
#define _tdbgprintf  _wdbgprintf
#else
#define _tdbgprintf  _dbgprintf
#endif // !UNICODE
