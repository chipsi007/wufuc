#include <Windows.h>

#include "helpers.h"
#include "logging.h"
#include "hooks.h"

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    switch (ul_reason_for_call) {
    case DLL_PROCESS_ATTACH:
    {
        if (!IsOperatingSystemSupported() || IsWow64())
            return FALSE;
        
        DisableThreadLibraryCalls(hModule);
        HANDLE hThread = CreateThread(NULL, 0, NewThreadProc, NULL, 0, NULL);
        CloseHandle(hThread);
        break;
    }
    case DLL_PROCESS_DETACH:
        logging_free();
        break;
    default:
        break;
    }
    return TRUE;
}
