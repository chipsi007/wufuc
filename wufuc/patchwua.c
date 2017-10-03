#include "patchwua.h"

#include "helpers.h"
#include "patternfind.h"
#include "tracing.h"

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include <phnt_windows.h>
#include <phnt.h>

bool patch_wua(void *lpBaseOfDll, size_t SizeOfImage)
{
        char *pattern;
        size_t offset1, offset2;
#ifdef _M_AMD64
        pattern = "FFF3 4883EC?? 33DB 391D???????? 7508 8B05????????";
        offset1 = 10;
        offset2 = 18;
#elif defined(_M_IX86)
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
        unsigned char *ptr = patternfind(lpBaseOfDll, SizeOfImage, pattern);
        if ( !ptr ) {
                trace(L"No pattern match! (couldn't patch)");
                return FALSE;
        }

        wchar_t path[MAX_PATH];
        GetModuleFileName((HMODULE)lpBaseOfDll, path, _countof(path));
        wchar_t *fname = find_fname(path);
        trace(L"Matched pattern at %ls!%p", 
                fname,
                ptr);
        LPBOOL lpb1, lpb2;
#ifdef _M_AMD64
        lpb1 = (LPBOOL)(ptr + offset1 + sizeof(uint32_t) + *(uint32_t *)(ptr + offset1));
        lpb2 = (LPBOOL)(ptr + offset2 + sizeof(uint32_t) + *(uint32_t *)(ptr + offset2));
#elif defined(_M_IX86)
        lpb1 = (LPBOOL)(*(uintptr_t *)(ptr + offset1));
        lpb2 = (LPBOOL)(*(uintptr_t *)(ptr + offset2));
#endif

        DWORD flOldProtect;
        if ( *lpb1 == TRUE ) {
                if ( VirtualProtect(lpb1, sizeof(BOOL), PAGE_READWRITE, &flOldProtect) ) {
                        *lpb1 = FALSE;
                        trace(L"Patched value #1 at %ls!%p: %08X", fname, lpb1, *lpb1);
                        if ( !VirtualProtect(lpb1, sizeof(BOOL), flOldProtect, &flOldProtect) )
                                trace(L"Failed to restore memory region permissions at %ls!%p (error code=%08X)", fname, lpb1, GetLastError());
                } else trace(L"Failed to change memory region permissions at %ls!%p (error code=%08X)", fname, lpb1, GetLastError());
        }
        if ( *lpb2 == FALSE ) {
                if ( VirtualProtect(lpb2, sizeof(BOOL), PAGE_READWRITE, &flOldProtect) ) {
                        *lpb2 = TRUE;
                        trace(L"Patched value #2 at %ls!%p: %08X", fname, lpb2, *lpb2);
                        if ( !VirtualProtect(lpb2, sizeof(BOOL), flOldProtect, &flOldProtect) )
                                trace(L"Failed to restore memory region permissions at %ls!%p: (error code=%08X)", fname, lpb2, GetLastError());
                } else trace(L"Failed to change memory region permissions at %ls!%p (error code=%08X)", fname, lpb2, GetLastError());
        }
        return !*lpb1 && *lpb2;
}

