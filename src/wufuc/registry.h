#pragma once

DWORD RegGetValueAlloc(PVOID *ppvData, HKEY hKey, LPCWSTR lpSubKey, LPCWSTR lpValueName, DWORD dwFlags, LPDWORD pdwType);
