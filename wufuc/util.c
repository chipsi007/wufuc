#include <Windows.h>
#include <stdio.h>
#include <TlHelp32.h>
#include <tchar.h>
#include "util.h"

VOID DetourIAT(HMODULE hModule, LPSTR lpFuncName, LPVOID *lpOldAddress, LPVOID lpNewAddress) {
    LPVOID *lpAddress = FindIAT(hModule, lpFuncName);
    if (!lpAddress || *lpAddress == lpNewAddress) {
        return;
    }

    DWORD flOldProtect;
    DWORD flNewProtect = PAGE_READWRITE;
    VirtualProtect(lpAddress, sizeof(LPVOID), flNewProtect, &flOldProtect);
    if (lpOldAddress) {
        *lpOldAddress = *lpAddress;
    }
    _dbgprintf("Detoured %s from %p to %p.", lpFuncName, *lpAddress, lpNewAddress);
    *lpAddress = lpNewAddress;
    VirtualProtect(lpAddress, sizeof(LPVOID), flOldProtect, &flNewProtect);
}

LPVOID *FindIAT(HMODULE hModule, LPSTR lpFunctionName) {
    SIZE_T hm = (SIZE_T)hModule;

    for (PIMAGE_IMPORT_DESCRIPTOR iid = (PIMAGE_IMPORT_DESCRIPTOR)(hm + ((PIMAGE_NT_HEADERS)(hm + ((PIMAGE_DOS_HEADER)hm)->e_lfanew))
        ->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress); iid->Name; iid++) {

        LPVOID *p;
        for (SIZE_T i = 0; *(p = i + (LPVOID *)(hm + iid->FirstThunk)); i++) {
            LPSTR fn = (LPSTR)(hm + *(i + (SIZE_T *)(hm + iid->OriginalFirstThunk)) + 2);
            if (!((uintptr_t)fn & IMAGE_ORDINAL_FLAG) && !_stricmp(lpFunctionName, fn)) {
                return p;
            }
        }
    }
    return NULL;
}

VOID SuspendProcessThreads(DWORD dwProcessId, DWORD dwThreadId, HANDLE *lphThreads, SIZE_T dwSize, SIZE_T *lpcb) {
    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
    THREADENTRY32 te;
    te.dwSize = sizeof(te);
    Thread32First(hSnap, &te);

    SIZE_T count = 0;

    do {
        if (te.th32OwnerProcessID != dwProcessId || te.th32ThreadID == dwThreadId) {
            continue;
        }
        lphThreads[count] = OpenThread(THREAD_SUSPEND_RESUME, FALSE, te.th32ThreadID);
        SuspendThread(lphThreads[count]);
        count++;
    } while (count < dwSize && Thread32Next(hSnap, &te));
    CloseHandle(hSnap);

    *lpcb = count;
    _tdbgprintf(_T("Suspended %d other threads."), count);
}

VOID ResumeAndCloseThreads(HANDLE *lphThreads, SIZE_T cb) {
    for (SIZE_T i = 0; i < cb; i++) {
        ResumeThread(lphThreads[i]);
        CloseHandle(lphThreads[i]);
    }
    _tdbgprintf(_T("Resumed %d other threads."), cb);
}

BOOL WindowsVersionCompare(BYTE Operator, DWORD dwMajorVersion, DWORD dwMinorVersion, WORD wServicePackMajor, WORD wServicePackMinor, DWORD dwTypeMask) {
    OSVERSIONINFOEX osvi;
    ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));
    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
    osvi.dwMajorVersion = dwMajorVersion;
    osvi.dwMinorVersion = dwMinorVersion;
    osvi.wServicePackMajor = wServicePackMajor;
    osvi.wServicePackMinor = wServicePackMinor;

    DWORDLONG dwlConditionMask = 0;
    VER_SET_CONDITION(dwlConditionMask, VER_MAJORVERSION, Operator);
    VER_SET_CONDITION(dwlConditionMask, VER_MINORVERSION, Operator);
    VER_SET_CONDITION(dwlConditionMask, VER_SERVICEPACKMAJOR, Operator);
    VER_SET_CONDITION(dwlConditionMask, VER_SERVICEPACKMINOR, Operator);

    return VerifyVersionInfo(&osvi, dwTypeMask, dwlConditionMask);
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
