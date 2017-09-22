#ifndef LOGGING_H_INCLUDED
#define LOGGING_H_INCLUDED
#pragma once

#include <Windows.h>
#include <tchar.h>

extern IMAGE_DOS_HEADER __ImageBase;
#define HINST_THISCOMPONENT ((HINSTANCE)&__ImageBase)

BOOL InitTracing(void);
DWORD WaitForTracingMutex(void);
BOOL ReleaseTracingMutex(void);
void trace_(LPCTSTR format, ...);
BOOL DeinitTracing(void);

#define STRINGIZE_(x) #x
#define STRINGIZE(x) STRINGIZE_(x)

#define LINESTR STRINGIZE(__LINE__)
#define trace(format, ...) trace_(_T(__FILE__) _T(":") _T(LINESTR) _T("(") _T(__FUNCTION__) _T("): ") format _T("\n"), ##__VA_ARGS__)
#endif
