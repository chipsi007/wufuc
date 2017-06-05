#pragma once

EXTERN_C IMAGE_DOS_HEADER __ImageBase;
#define HINST_THISCOMPONENT ((HINSTANCE)&__ImageBase)

BOOL IsWindows7Or8Point1(void);

BOOL IsWindows7(void);

BOOL IsWindows8Point1(void);

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

#ifdef UNICODE
#define CommandLineToArgv  CommandLineToArgvW
#else
#define CommandLineToArgv  CommandLineToArgvA
#endif // !UNICODE
