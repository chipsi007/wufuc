#ifndef HELPERS_H_INCLUDED
#define HELPERS_H_INCLUDED
#pragma once

#include <Windows.h>

typedef BOOL(WINAPI *ISWOW64PROCESS)(HANDLE, PBOOL);

BOOL IsWindows7(void);
BOOL IsWindows8Point1(void);
BOOL IsOperatingSystemSupported(void);
BOOL IsWow64(void);

size_t suspend_other_threads(LPHANDLE lphThreads, size_t nMaxCount);
void resume_and_close_threads(LPHANDLE lphThreads, size_t nCount);

char *get_cpuid_brand(char *brand, size_t nMaxCount);
wchar_t *find_fname(wchar_t *pPath);
int compare_versions(WORD ma1, WORD mi1, WORD b1, WORD r1, WORD ma2, WORD mi2, WORD b2, WORD r2);
#endif
