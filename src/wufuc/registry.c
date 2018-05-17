#include "stdafx.h"
#include "registry.h"

DWORD RegGetValueAlloc(PVOID *ppvData, HKEY hKey, LPCWSTR lpSubKey, LPCWSTR lpValueName, DWORD dwFlags, LPDWORD pdwType)
{
        DWORD result;
        PVOID pvData;

        if ( RegGetValueW(hKey, lpSubKey, lpValueName, dwFlags, pdwType, NULL, &result) != ERROR_SUCCESS )
                return 0;

        pvData = malloc(result);
        if ( !pvData ) return 0;

        if ( RegGetValueW(hKey, lpSubKey, lpValueName, dwFlags, pdwType, pvData, &result) == ERROR_SUCCESS ) {
                *ppvData = pvData;
        } else {
                free(pvData);
                return 0;
        }
        return result;
}
