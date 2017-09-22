#include "callbacks.h"

#include "iathook.h"
#include "hooks.h"
#include "helpers.h"
#include "service.h"
#include "tracing.h"

#include <windows.h>
#include <tchar.h>
#include <sddl.h>

DWORD WINAPI ThreadProcCallback(LPVOID *lpParam) {
    OSVERSIONINFO osvi = { 0 };
    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
#pragma warning(suppress : 4996)
    if ( GetVersionEx(&osvi) )
        trace(_T("Windows version: %d.%d.%d (%zu-bit)"), osvi.dwMajorVersion, osvi.dwMinorVersion, osvi.dwBuildNumber, sizeof(uintptr_t) * 8);
    else trace(_T("Failed to get Windows version (error code=%08X)"), GetLastError());

    char brand[0x31];
    trace(_T("Processor: %hs"), get_cpuid_brand(brand, _countof(brand)));

    LPTSTR lpCommandLine = GetCommandLine();
    TCHAR lpServiceCommandLine[0x8000];
    if ( GetServiceCommandLine(NULL, _T("wuauserv"), lpServiceCommandLine, _countof(lpServiceCommandLine))
        && !_tcsicmp(lpCommandLine, lpServiceCommandLine) ) {

        SECURITY_ATTRIBUTES sa = { 0 };
        sa.nLength = sizeof(SECURITY_ATTRIBUTES);
        sa.bInheritHandle = FALSE;
        if ( ConvertStringSecurityDescriptorToSecurityDescriptor(_T("D:(A;;0x001F0003;;;BA)"), SDDL_REVISION_1, &sa.lpSecurityDescriptor, NULL) ) {
            HANDLE hEvent = CreateEvent(&sa, TRUE, FALSE, _T("Global\\wufuc_UnloadEvent"));
            if ( hEvent ) {
                HANDLE threads[0x1000];

                size_t count = suspend_other_threads(threads, _countof(threads));
                iat_hook(GetModuleHandle(NULL), "RegQueryValueExW", (LPVOID)&pfnRegQueryValueExW, RegQueryValueExW_hook);
                iat_hook(GetModuleHandle(NULL), "LoadLibraryExW", (LPVOID)&pfnLoadLibraryExW, LoadLibraryExW_hook);
                resume_and_close_threads(threads, count);

                WaitForSingleObject(hEvent, INFINITE);
                trace(_T("Unload event was set, removing hooks and unloading..."));

                count = suspend_other_threads(threads, _countof(threads));
                iat_hook(GetModuleHandle(NULL), "LoadLibraryExW", NULL, pfnLoadLibraryExW);
                iat_hook(GetModuleHandle(NULL), "RegQueryValueExW", NULL, pfnRegQueryValueExW);
                resume_and_close_threads(threads, count);
                CloseHandle(hEvent);
            } else trace(_T("Failed to create unload event (error code=%08X)"), GetLastError());
        } else trace(_T("Failed to convert string security descriptor to security descriptor (error code=%08X)"), GetLastError());
    } else trace(_T("Current process command line is incorrect: %s != %s"), lpCommandLine, lpServiceCommandLine);
    trace(_T("Bye bye!"));
    FreeLibraryAndExitThread((HMODULE)lpParam, 0);
}
