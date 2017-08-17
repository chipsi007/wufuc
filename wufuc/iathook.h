#pragma once

VOID iat_hook(HMODULE hModule, LPCSTR lpFuncName, LPVOID *lpOldAddress, LPVOID lpNewAddress);

static LPVOID *iat_find(HMODULE hModule, LPCSTR lpFunctionName);
