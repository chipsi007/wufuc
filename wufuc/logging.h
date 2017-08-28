#ifndef LOGGING_H
#define LOGGING_H
#pragma once

#include <Windows.h>
#include <tchar.h>

extern IMAGE_DOS_HEADER __ImageBase;
#define HINST_THISCOMPONENT ((HINSTANCE)&__ImageBase)

BOOL InitLogging(void);
void trace_(LPCTSTR format, ...);
BOOL FreeLogging(void);

#define STRINGIZE_(x) #x
#define STRINGIZE(x) STRINGIZE_(x)

#define __LINESTR__ STRINGIZE(__LINE__)
#define trace(format, ...) trace_(_T(__FILE__) _T(":") _T(__LINESTR__) _T("(") _T(__FUNCTION__) _T("): ") format _T("\n"), ##__VA_ARGS__)
#endif
