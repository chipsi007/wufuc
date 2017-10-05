#include "patchwua.h"

#include "helpers.h"
#include "patternfind.h"
#include "tracing.h"

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include <phnt_windows.h>
#include <phnt.h>

#ifdef _M_AMD64
static const PatchSet X64PatchSet = { "FFF3 4883EC?? 33DB 391D???????? 7508 8B05????????", 0xA, 0x12 };
#elif defined(_M_IX86)
static const PatchSet Win7X86PatchSet = { "833D????????00 743E E8???????? A3????????", 0x2, 0xF };
static const PatchSet Win81X86PatchSet = { "8BFF 51 833D????????00 7507 A1????????", 0x5, 0xD };
#endif

bool calculate_pointers(uintptr_t lpfn, const PatchSet *ps, LPBOOL *ppba, LPBOOL *ppbb)
{
#ifdef _M_AMD64
        *ppba = (LPBOOL)(lpfn + ps->Offset1 + sizeof(uint32_t) + *(uint32_t *)(lpfn + ps->Offset1));
        *ppbb = (LPBOOL)(lpfn + ps->Offset2 + sizeof(uint32_t) + *(uint32_t *)(lpfn + ps->Offset2));
        return true;
#elif defined(_M_IX86)
        *ppba = (LPBOOL)(*(uintptr_t *)(lpfn + ps->Offset1));
        *ppbb = (LPBOOL)(*(uintptr_t *)(lpfn + ps->Offset2));
        return true;
#endif
        return false;
}

bool patch_wua(void *lpBaseOfDll, size_t SizeOfImage, wchar_t *fname)
{
        bool result = false;

        const PatchSet *pps;
#ifdef _M_AMD64
        pps = &X64PatchSet;
#elif defined(_M_IX86)
        if ( verify_winver(6, 1, 0, 0, 0, VER_EQUAL, VER_EQUAL, 0, 0, 0) )
                pps = &Win7X86PatchSet;
        else if ( verify_winver(6, 3, 0, 0, 0, VER_EQUAL, VER_EQUAL, 0, 0, 0) )
                pps = &Win81X86PatchSet;
#endif
        unsigned char *ptr = patternfind(lpBaseOfDll, SizeOfImage, pps->Pattern);
        if ( !ptr ) {
                trace(L"No pattern match! (couldn't patch)");
                goto L_ret;
        }

        LPBOOL pba, pbb;
        if ( calculate_pointers((uintptr_t)ptr, pps, &pba, &pbb) ) {
                DWORD flOldProtect;
                if ( *pba == TRUE ) {
                        if ( VirtualProtect(pba, sizeof(BOOL), PAGE_READWRITE, &flOldProtect) ) {
                                *pba = FALSE;
                                trace(L"Patched value a at %ls!%p: %08X", fname, pba, *pba);
                                if ( !VirtualProtect(pba, sizeof(BOOL), flOldProtect, &flOldProtect) )
                                        trace(L"Failed to restore memory region permissions at %ls!%p (error code=%08X)", fname, pba, GetLastError());
                        } else trace(L"Failed to change memory region permissions at %ls!%p (error code=%08X)", fname, pba, GetLastError());
                }
                if ( *pbb == FALSE ) {
                        if ( VirtualProtect(pbb, sizeof(BOOL), PAGE_READWRITE, &flOldProtect) ) {
                                *pbb = TRUE;
                                trace(L"Patched value b at %ls!%p: %08X", fname, pbb, *pbb);
                                if ( !VirtualProtect(pbb, sizeof(BOOL), flOldProtect, &flOldProtect) )
                                        trace(L"Failed to restore memory region permissions at %ls!%p: (error code=%08X)", fname, pbb, GetLastError());
                        } else trace(L"Failed to change memory region permissions at %ls!%p (error code=%08X)", fname, pbb, GetLastError());
                }
                result = !*pba && *pbb;
        }
L_ret:
        return result;
}
