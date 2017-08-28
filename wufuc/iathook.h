#ifndef IATHOOK_H
#define IATHOOK_H
#pragma once

#include <Windows.h>

void iat_hook(HMODULE hModule, LPCSTR lpFuncName, LPVOID *lpOldAddress, LPVOID lpNewAddress);
#endif
