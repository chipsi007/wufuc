#pragma once

typedef struct
{
        WORD wLanguage;
        WORD wCodePage;
} LANGANDCODEPAGE, *PLANGANDCODEPAGE;

DWORD GetModuleVersionInfo(HMODULE hModule, LPVOID *lplpData);

DWORD GetFileVersionInfoExAlloc(DWORD dwFlags, BOOL bPrefetched, LPCWSTR lpwstrFilename, LPVOID *lplpData);

UINT VerQueryString(LPCVOID pBlock, LANGANDCODEPAGE LangCodePage, LPCWSTR lpName, LPWSTR *lplpString);

UINT VerQueryTranslations(LPCVOID pBlock, PLANGANDCODEPAGE *lplpTranslate);

UINT VerQueryFileInfo(LPCVOID pBlock, VS_FIXEDFILEINFO **lplpffi);

int vercmp(DWORD dwMS, DWORD dwLS, WORD wMajor, WORD wMinor, WORD wBuild, WORD wRev);
