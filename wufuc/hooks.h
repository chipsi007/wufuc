#pragma once

typedef HMODULE(WINAPI *LOADLIBRARYEXA)(LPCSTR, HANDLE, DWORD);
typedef HMODULE(WINAPI *LOADLIBRARYEXW)(LPCWSTR, HANDLE, DWORD);

extern LOADLIBRARYEXW fpLoadLibraryExW;
extern LOADLIBRARYEXA fpLoadLibraryExA;

DWORD WINAPI NewThreadProc(LPVOID lpParam);

HMODULE WINAPI LoadLibraryExA_hook(
    _In_       LPCSTR  lpFileName,
    _Reserved_ HANDLE  hFile,
    _In_       DWORD   dwFlags
);

HMODULE WINAPI LoadLibraryExW_hook(
    _In_       LPCWSTR lpFileName,
    _Reserved_ HANDLE  hFile,
    _In_       DWORD   dwFlags
);

BOOL PatchWUA(HMODULE hModule);
