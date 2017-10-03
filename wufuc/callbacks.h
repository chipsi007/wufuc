#pragma once

#include <phnt_windows.h>

VOID NTAPI VerifierDllLoadCallback(PWSTR DllName, PVOID DllBase, SIZE_T DllSize, PVOID Reserved);
VOID NTAPI VerifierDllUnloadCallback(PWSTR DllName, PVOID DllBase, SIZE_T DllSize, PVOID Reserved);
