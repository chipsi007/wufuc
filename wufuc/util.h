#pragma once

EXTERN_C IMAGE_DOS_HEADER __ImageBase;
#define HINST_THISCOMPONENT ((HINSTANCE)&__ImageBase)

BOOL IsWindows7Or8Point1(void);

BOOL IsWindows7(void);

BOOL IsWindows8Point1(void);

//#ifdef _DEBUG
VOID _DbgPrint(LPCTSTR format, ...);
#define DbgPrint(format, ...) \
	_DbgPrint(_T(__FUNCTION__) _T(": ") _T(format), ##__VA_ARGS__)
//#else
//#define DbgPrint(format, ...)
//#endif

#ifdef UNICODE
#define CommandLineToArgv  CommandLineToArgvW
#else
#define CommandLineToArgv  CommandLineToArgvA
#endif // !UNICODE
