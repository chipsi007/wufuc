#include "stdafx.h"
#include "versioninfo.h"

DWORD GetModuleVersionInfo(HMODULE hModule, LPVOID *lplpData)
{
        HRSRC hResInfo;
        DWORD result;
        HGLOBAL hResData;
        LPVOID pRes;
        LPVOID lpData;

        hResInfo = FindResourceW(hModule,
                MAKEINTRESOURCEW(VS_VERSION_INFO),
                MAKEINTRESOURCEW(RT_VERSION));
        if ( !hResInfo ) return 0;

        result = SizeofResource(hModule, hResInfo);
        if ( !result ) return 0;

        hResData = LoadResource(hModule, hResInfo);
        if ( !hResData ) return 0;

        pRes = LockResource(hResData);
        if ( !pRes ) return 0;

        lpData = malloc(result);
        if ( !lpData ) return 0;

        if ( !memcpy_s(lpData, result, pRes, result) ) {
                *lplpData = lpData;
        } else {
                free(lpData);
                return 0;
        }
        return result;
}

DWORD GetFileVersionInfoExAlloc(DWORD dwFlags, BOOL bPrefetched, LPCWSTR lpwstrFilename, LPVOID *lplpData)
{
        DWORD result;
        DWORD dwHandle;
        LPVOID lpData;

        result = GetFileVersionInfoSizeExW(dwFlags,
                lpwstrFilename,
                &dwHandle);
        if ( !result ) return 0;

        lpData = malloc(result);
        if ( !lpData ) return 0;

        if ( GetFileVersionInfoExW(bPrefetched ? (dwFlags | FILE_VER_GET_PREFETCHED) : dwFlags,
                lpwstrFilename,
                dwHandle,
                result,
                lpData) ) {
                *lplpData = lpData;
        } else {
                free(lpData);
                return 0;
        }
        return result;
}

UINT VerQueryString(LPCVOID pBlock, LANGANDCODEPAGE LangCodePage, LPCWSTR lpName, LPWSTR *lplpString)
{
        LPWSTR pszSubBlock;
        LPVOID lpBuffer;
        UINT result = 0;

        if ( aswprintf(&pszSubBlock,
                L"\\StringFileInfo\\%04x%04x\\%ls",
                LangCodePage.wLanguage,
                LangCodePage.wCodePage,
                lpName) == -1 )
                return 0;

        if ( VerQueryValueW(pBlock, pszSubBlock, &lpBuffer, &result) )
                *lplpString = (LPWSTR)lpBuffer;

        free(pszSubBlock);
        return result;
}

UINT VerQueryTranslations(LPCVOID pBlock, PLANGANDCODEPAGE *lplpTranslate)
{
        PLANGANDCODEPAGE lpTranslate;
        UINT uLen;

        if ( VerQueryValueW(pBlock, L"\\VarFileInfo\\Translation", &(LPVOID)lpTranslate, &uLen) ) {
                *lplpTranslate = lpTranslate;
                return uLen / (sizeof *lpTranslate);
        }
        return 0;
}

UINT VerQueryFileInfo(LPCVOID pBlock, VS_FIXEDFILEINFO **lplpffi)
{
        VS_FIXEDFILEINFO *lpffi;
        UINT result;

        if ( VerQueryValueW(pBlock, L"\\", &(LPVOID)lpffi, &result) ) {
                *lplpffi = lpffi;
                return result;
        }
        return 0;
}

int vercmp(DWORD dwMS, DWORD dwLS, WORD wMajor, WORD wMinor, WORD wBuild, WORD wRev)
{
        WORD w;

        w = HIWORD(dwMS);
        if ( w < wMajor ) return -1;
        if ( w > wMajor ) return 1;

        w = LOWORD(dwMS);
        if ( w < wMinor ) return -1;
        if ( w > wMinor ) return 1;

        w = HIWORD(dwLS);
        if ( w < wBuild ) return -1;
        if ( w > wBuild ) return 1;

        w = LOWORD(dwLS);
        if ( w < wRev ) return -1;
        if ( w > wRev ) return 1;

        return 0;
}
