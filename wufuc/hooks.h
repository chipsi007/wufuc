#pragma once

#include <phnt_windows.h>

typedef struct tagLANGANDCODEPAGE
{
        WORD wLanguage;
        WORD wCodePage;
} LANGANDCODEPAGE, *PLANGANDCODEPAGE;

LSTATUS WINAPI RegQueryValueExW_Hook(HKEY hKey, LPCWSTR lpValueName, LPDWORD lpReserved, LPDWORD lpType, LPBYTE lpData, LPDWORD lpcbData);
HMODULE WINAPI LoadLibraryExW_Hook(LPCWSTR lpFileName, HANDLE hFile, DWORD dwFlags);
