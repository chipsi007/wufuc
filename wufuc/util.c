#include <Windows.h>
#include <stdio.h>
#include <VersionHelpers.h>
#include <tchar.h>
#include "util.h"

BOOL IsWindows7Or8Point1(void) {
	return IsWindows7() || IsWindows8Point1();
}

BOOL IsWindows7(void) {
	return IsWindows7OrGreater() && !IsWindows8OrGreater();
}

BOOL IsWindows8Point1(void) {
	return IsWindows8Point1OrGreater() && !IsWindows10OrGreater();
}

VOID _wdbgprintf(LPCWSTR format, ...) {
	WCHAR buffer[0x1000];
	va_list argptr;
	va_start(argptr, format);
	vswprintf_s(buffer, _countof(buffer), format, argptr);
	va_end(argptr);
	OutputDebugStringW(buffer);
}

VOID _dbgprintf(LPCSTR format, ...) {
	CHAR buffer[0x1000];
	va_list argptr;
	va_start(argptr, format);
	vsprintf_s(buffer, _countof(buffer), format, argptr);
	va_end(argptr);
	OutputDebugStringA(buffer);
}
