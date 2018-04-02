#pragma once

typedef struct
{
        WORD wLanguage;
        WORD wCodePage;
} LANGANDCODEPAGE, *PLANGANDCODEPAGE;

void *res_get_version_info(HMODULE hModule);

wchar_t *res_query_string_file_info(const void *pBlock,
        LANGANDCODEPAGE lcp,
        const wchar_t *pszStringName,
        size_t *pcchLength);

PLANGANDCODEPAGE res_query_var_file_info(const void *pBlock, size_t *pcbData);

VS_FIXEDFILEINFO *res_query_fixed_file_info(const void *pBlock);
