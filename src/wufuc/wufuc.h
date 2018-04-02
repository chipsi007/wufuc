#pragma once

typedef struct _PATCHINFO
{
        const char *pattern;
        size_t off1;
        size_t off2;
} PATCHINFO;

#define SVCHOST_CRASH_THRESHOLD 3
extern HANDLE g_hMainMutex;

bool wufuc_inject(DWORD dwProcessId,
        LPTHREAD_START_ROUTINE pStartAddress,
        ptrlist_t *list);
void wufuc_patch(HMODULE hModule);
