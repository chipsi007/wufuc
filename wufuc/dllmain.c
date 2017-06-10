#include <Windows.h>
#include "core.h"
#include "util.h"

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    switch (ul_reason_for_call) {
    case DLL_PROCESS_ATTACH:
    {
        DisableThreadLibraryCalls(hModule);
        if (WindowsVersionCompare(VER_EQUAL, 6, 1, 0, 0, VER_MAJORVERSION | VER_MINORVERSION)
            || WindowsVersionCompare(VER_EQUAL, 6, 3, 0, 0, VER_MAJORVERSION | VER_MINORVERSION)) {

            HANDLE hThread = CreateThread(NULL, 0, NewThreadProc, NULL, 0, NULL);
            CloseHandle(hThread);
        }
        break;
    }
    case DLL_PROCESS_DETACH:
        break;
    default:
        break;
    }
    return TRUE;
}
