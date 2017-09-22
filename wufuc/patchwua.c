#include "patchwua.h"

#include "helpers.h"
#include "patternfind.h"
#include "tracing.h"

#include <stdint.h>

#include <Windows.h>
#include <tchar.h>
#include <Psapi.h>

BOOL PatchWUA(void *lpBaseOfDll, size_t SizeOfImage) {
    char *pattern;
    size_t offset1, offset2;
#ifdef _AMD64_
    pattern = "FFF3 4883EC?? 33DB 391D???????? 7508 8B05????????";
    offset1 = 10;
    offset2 = 18;
#elif defined(_X86_)
    if ( IsWindows7() ) {
        pattern = "833D????????00 743E E8???????? A3????????";
        offset1 = 2;
        offset2 = 15;
    } else if ( IsWindows8Point1() ) {
        pattern = "8BFF 51 833D????????00 7507 A1????????";
        offset1 = 5;
        offset2 = 13;
    }
#endif

    unsigned char *ptr = patternfind(lpBaseOfDll, SizeOfImage, 0, pattern);
    if ( !ptr ) {
        trace(_T("No pattern match! (couldn't patch)"));
        return FALSE;
    }
    TCHAR fname[MAX_PATH];
    GetModuleBaseName(GetCurrentProcess(), (HMODULE)lpBaseOfDll, fname, _countof(fname));
    trace(_T("Matched pattern for IsDeviceServiceable at %s+0x%zx"), fname,
        (size_t)((uintptr_t)ptr - (uintptr_t)lpBaseOfDll));
    LPBOOL lpb1, lpb2;
#ifdef _AMD64_
    lpb1 = (LPBOOL)(ptr + offset1 + sizeof(uint32_t) + *(uint32_t *)(ptr + offset1));
    lpb2 = (LPBOOL)(ptr + offset2 + sizeof(uint32_t) + *(uint32_t *)(ptr + offset2));
#elif defined(_X86_)
    lpb1 = (LPBOOL)(*(uintptr_t *)(ptr + offset1));
    lpb2 = (LPBOOL)(*(uintptr_t *)(ptr + offset2));
#endif
    offset1 = (size_t)((uintptr_t)lpb1 - (uintptr_t)lpBaseOfDll);
    offset2 = (size_t)((uintptr_t)lpb2 - (uintptr_t)lpBaseOfDll);

    DWORD flOldProtect;
    if ( *lpb1 ) {
        if ( VirtualProtect(lpb1, sizeof(BOOL), PAGE_READWRITE, &flOldProtect) ) {
            *lpb1 = FALSE;
            trace(_T("Patched value #1 at %s+0x%zx: %08x"), fname, offset1, *lpb1);
            if ( !VirtualProtect(lpb1, sizeof(BOOL), flOldProtect, &flOldProtect) )
                trace(_T("Failed to restore memory region permissions at %s+0x%zx (error code=%08x)"), fname, offset1, GetLastError());
        } else trace(_T("Failed to change memory region permissions at %s+0x%zx (error code=%08x)"), fname, offset1, GetLastError());
    }
    if ( !*lpb2 ) {
        if ( VirtualProtect(lpb2, sizeof(BOOL), PAGE_READWRITE, &flOldProtect) ) {
            *lpb2 = TRUE;
            trace(_T("Patched value #2 at %s+0x%zx: %08x"), fname, offset2, *lpb2);
            if ( !VirtualProtect(lpb2, sizeof(BOOL), flOldProtect, &flOldProtect) )
                trace(_T("Failed to restore memory region permissions at %s+0x%zx (error code=%08x)"), fname, offset2, GetLastError());
        } else trace(_T("Failed to change memory region permissions at %s+0x%zx (error code=%08x)"), fname, offset2, GetLastError());
    }
    return !*lpb1 && *lpb2;
}
