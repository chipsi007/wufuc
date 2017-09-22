#include "callbacks.h"
#include "helpers.h"
#include "ntdllhelper.h"
#include "tracing.h"

#include <Windows.h> 

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    switch ( ul_reason_for_call ) {
    case DLL_PROCESS_ATTACH:
    {
        if ( !IsOperatingSystemSupported() /*|| IsWow64()*/ )
            return FALSE;

        HANDLE hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ThreadProcCallback, (LPVOID)hModule, 0, NULL);
        CloseHandle(hThread);
        break;
    }
    case DLL_PROCESS_DETACH:
        DeinitTracing();
        break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
        break;
    }
    return TRUE;
}


