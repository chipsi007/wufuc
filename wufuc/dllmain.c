#include <Windows.h>
#include "core.h"
#include "util.h"
#include "shared.h"

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    switch (ul_reason_for_call) {
    case DLL_PROCESS_ATTACH:
    {
        if (!IsOperatingSystemSupported(&g_IsWindows7, &g_IsWindows8Point1)) {
            return FALSE;
        }

        DisableThreadLibraryCalls(hModule);
        HANDLE hThread = CreateThread(NULL, 0, NewThreadProc, NULL, 0, NULL);
        CloseHandle(hThread);
        break;
    }
    case DLL_PROCESS_DETACH:
        break;
    default:
        break;
    }
    return TRUE;
}
