#include "stdafx.h"
#include "resourcehelper.h"
#include "log.h"

void *res_get_version_info(HMODULE hModule)
{
        HRSRC hResInfo;
        DWORD dwSize;
        HGLOBAL hResData;
        LPVOID pRes;
        void *result;

        hResInfo = FindResourceW(hModule,
                MAKEINTRESOURCEW(VS_VERSION_INFO),
                MAKEINTRESOURCEW(RT_VERSION));
        if ( !hResInfo ) return NULL;

        dwSize = SizeofResource(hModule, hResInfo);
        if ( !dwSize ) return NULL;

        hResData = LoadResource(hModule, hResInfo);
        if ( !hResData ) return NULL;

        pRes = LockResource(hResData);
        if ( !pRes ) return NULL;

        result = malloc(dwSize);
        if ( !result ) return NULL;

        if ( memcpy_s(result, dwSize, pRes, dwSize) ) {
                free(result);
                result = NULL;
        }
        return result;
}

wchar_t *res_query_string_file_info(const void *pBlock,
        LANGANDCODEPAGE lcp,
        const wchar_t *pszStringName,
        size_t *pcchLength)
{
        const wchar_t fmt[] = L"\\StringFileInfo\\%04x%04x\\%ls";
        int ret;
        int count;
        wchar_t *pszSubBlock;
        wchar_t *result = NULL;
        UINT uLen;

        ret = _scwprintf(fmt, lcp.wLanguage, lcp.wCodePage, pszStringName);
        if ( ret == -1 ) return NULL;

        count = ret + 1;
        pszSubBlock = calloc(count, sizeof *pszSubBlock);
        if ( !pszSubBlock ) return NULL;

        ret = swprintf_s(pszSubBlock, count, fmt, lcp.wLanguage, lcp.wCodePage, pszStringName);
        if ( ret != -1
                && VerQueryValueW(pBlock, pszSubBlock, &(LPVOID)result, &uLen)
                && pcchLength )
                *pcchLength = uLen;

        free(pszSubBlock);
        return result;
}

PLANGANDCODEPAGE res_query_var_file_info(const void *pBlock, size_t *pCount)
{
        PLANGANDCODEPAGE result = NULL;
        UINT uLen;

        if ( VerQueryValueW(pBlock, L"\\VarFileInfo\\Translation", &(LPVOID)result, &uLen) )
                *pCount = uLen / (sizeof *result);
        return result;
}

VS_FIXEDFILEINFO *res_query_fixed_file_info(const void *pBlock)
{
        VS_FIXEDFILEINFO *result;
        UINT uLen;

        if ( VerQueryValueW(pBlock, L"\\", &(LPVOID)result, &uLen) )
                return result;
        return NULL;
}
