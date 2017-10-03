#include "tracing.h"

#include "helpers.h"
#include "rtl_malloc.h"

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#include <phnt_windows.h>
#include <phnt.h>

void trace_sysinfo(void)
{
        RTL_OSVERSIONINFOW osvi = { 0 };
        osvi.dwOSVersionInfoSize = sizeof(RTL_OSVERSIONINFOW);
        NTSTATUS status = RtlGetVersion(&osvi);
        if ( NT_SUCCESS(status) ) {
                trace(L"Windows version: %d.%d.%d (%Iu-bit)",
                        osvi.dwMajorVersion,
                        osvi.dwMinorVersion,
                        osvi.dwBuildNumber,
                        sizeof(uintptr_t) * 8);
        } else trace(L"Failed to get Windows version (status=%08X)", status);
        
        int CPUInfo[4];
        __cpuidex(CPUInfo, 0x80000000, 0);
        if ( CPUInfo[0] < 0x80000004 ) {
                trace(L"This processor does not support the brand identification feature.");
                return;
        }
        char brand[0x31];
        uint32_t *u32ptr = (uint32_t *)&brand;
        for ( int func = 0x80000002; func <= 0x80000004; func++ ) {
                __cpuidex(CPUInfo, func, 0);
                for ( int i = 0; i < 4; i++ )
                        *(u32ptr++) = CPUInfo[i];
        }
        size_t c = 0;
        do {
                if ( !isspace(brand[c]) )
                        break;
                c++;
        } while ( c < _countof(brand) );
        trace(L"Processor: %hs", &brand[c]);
}

void trace_(const wchar_t *const format, ...)
{
        static int shown_sysinfo = 0;
        if ( !shown_sysinfo ) {
                shown_sysinfo = 1;
                trace_sysinfo();
        }
        va_list argptr;
        va_start(argptr, format);
        int count = _vscwprintf(format, argptr) + 1;
        wchar_t *buffer = rtl_calloc(count, sizeof(wchar_t));
        vswprintf_s(buffer, count, format, argptr);
        va_end(argptr);
        OutputDebugStringW(buffer);
        rtl_free(buffer);
}
