#include "callbacks.h"

#include "tracing.h"

#include <phnt_windows.h>

VOID NTAPI VerifierDllLoadCallback(PWSTR DllName, PVOID DllBase, SIZE_T DllSize, PVOID Reserved)
{
        trace(L"dll load %ls, DllBase=%p, DllSize=%Iu", DllName, DllBase, DllSize);
}

VOID NTAPI VerifierDllUnloadCallback(PWSTR DllName, PVOID DllBase, SIZE_T DllSize, PVOID Reserved)
{
        trace(L"dll unload %ls, DllBase=%p, DllSize=%Iu", DllName, DllBase, DllSize);
}
