#pragma once

EXTERN_C IMAGE_DOS_HEADER __ImageBase;
#define HINST_THISCOMPONENT ((HINSTANCE)&__ImageBase)

BOOL IsWindows7Or8Point1(void);

BOOL IsWindows7(void);

BOOL IsWindows8Point1(void);

VOID DetourIAT(HMODULE hModule, LPSTR lpFuncName, LPVOID *lpOldAddress, LPVOID lpNewAddress);

#define DETOUR_IAT(x, y) \
	LPVOID __LPORIGINAL##y; \
	DetourIAT(x, #y, &__LPORIGINAL##y, &_##y)

#define RESTORE_IAT(x, y) \
	DetourIAT(x, #y, NULL, __LPORIGINAL##y)

LPVOID *FindIAT(HMODULE hModule, LPSTR lpFuncName);

BOOL FindPattern(LPCBYTE lpBytes, SIZE_T nNumberOfBytes, LPSTR lpszPattern, SIZE_T nStart, SIZE_T *lpOffset);

BOOL InjectLibrary(HANDLE hProcess, LPCTSTR lpLibFileName, DWORD cb);

VOID SuspendProcessThreads(DWORD dwProcessId, DWORD dwThreadId, HANDLE *lphThreads, SIZE_T dwSize, SIZE_T *lpcb);

VOID ResumeAndCloseThreads(HANDLE *lphThreads, SIZE_T dwSize);

VOID _wdbgprintf(LPCWSTR format, ...);
VOID _dbgprintf(LPCSTR format, ...);
//#ifdef _DEBUG
#ifdef UNICODE
#define _tdbgprintf _wdbgprintf
#else
#define _tdbgprintf _dbgprintf
#endif // !UNICODE
//#else
//#define _tdbgprintf(format, ...)
//#endif // !_DEBUG
