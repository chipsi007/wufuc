#pragma once

typedef struct
{
        WORD wLanguage;
        WORD wCodePage;
} LANGANDCODEPAGE, *PLANGANDCODEPAGE;

#define SVCHOST_CRASH_THRESHOLD 3
extern HANDLE g_hMainMutex;

bool wufuc_inject(DWORD dwProcessId,
        LPTHREAD_START_ROUTINE pStartAddress,
        ptrlist_t *list);
bool wufuc_hook(HMODULE hModule);
