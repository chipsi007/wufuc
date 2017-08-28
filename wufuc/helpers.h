#ifndef HELPERS_H
#define HELPERS_H
#pragma once

#include <Windows.h>

typedef BOOL(WINAPI *ISWOW64PROCESS)(HANDLE, PBOOL);

BOOL IsWindows7(void);
BOOL IsWindows8Point1(void);
BOOL IsOperatingSystemSupported(void);
BOOL IsWow64(void);

void suspend_other_threads(LPHANDLE lphThreads, size_t *lpcb);
void resume_and_close_threads(LPHANDLE lphThreads, size_t cb);

void get_cpuid_brand(char *brand);

#define STRINGIZE_(x) #x
#define STRINGIZE(x) STRINGIZE_(x)
#endif
