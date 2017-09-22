#ifndef IATHOOK_H_INCLUDED
#define IATHOOK_H_INCLUDED
#pragma once

#include <Windows.h>

void iat_hook(HMODULE hModule, LPCSTR lpFuncName, LPVOID *lpOldAddress, LPVOID lpNewAddress);
#endif
