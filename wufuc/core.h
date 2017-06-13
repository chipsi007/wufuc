#pragma once

DWORD WINAPI NewThreadProc(LPVOID lpParam);

BOOL PatchWUAgentHMODULE(HMODULE hModule);

HMODULE WINAPI _LoadLibraryExA(
    _In_       LPCSTR  lpFileName,
    _Reserved_ HANDLE  hFile,
    _In_       DWORD   dwFlags
);

HMODULE WINAPI _LoadLibraryExW(
    _In_       LPCWSTR lpFileName,
    _Reserved_ HANDLE  hFile,
    _In_       DWORD   dwFlags
);
