#include "patchwua.h"

#include "helpers.h"
#include "patternfind.h"
#include "logging.h"

#include <stdint.h>

#include <Windows.h>
#include <tchar.h>
#include <Psapi.h>


BOOL PatchWUA(void *lpBaseOfDll, size_t SizeOfImage) {
    char *pattern;
    size_t offset00, offset01;
#ifdef _AMD64_
    pattern = "FFF3 4883EC?? 33DB 391D???????? 7508 8B05????????";
    offset00 = 10;
    offset01 = 18;
#elif defined(_X86_)
    if ( IsWindows7() ) {
        pattern = "833D????????00 743E E8???????? A3????????";
        offset00 = 2;
        offset01 = 15;
    } else if ( IsWindows8Point1() ) {
        pattern = "8BFF 51 833D????????00 7507 A1????????";
        offset00 = 5;
        offset01 = 13;
    }
#endif

    unsigned char *ptr = patternfind(lpBaseOfDll, SizeOfImage, 0, pattern);
    if ( !ptr ) {
        trace(_T("No pattern match!"));
        return FALSE;
    }
    trace(_T("wuaueng!IsDeviceServiceable VA: %p"), ptr);
    BOOL result = FALSE;
    LPBOOL lpbFirstRun, lpbIsCPUSupportedResult;
#ifdef _AMD64_
    lpbFirstRun = (LPBOOL)(ptr + offset00 + sizeof(uint32_t) + *(uint32_t *)(ptr + offset00));
    lpbIsCPUSupportedResult = (LPBOOL)(ptr + offset01 + sizeof(uint32_t) + *(uint32_t *)(ptr + offset01));
#elif defined(_X86_)
    lpbFirstRun = (LPBOOL)(*(uintptr_t *)(ptr + offset00));
    lpbIsCPUSupportedResult = (LPBOOL)(*(uintptr_t *)(ptr + offset01));
#endif

    DWORD flNewProtect = PAGE_READWRITE;
    DWORD flOldProtect;
    if ( *lpbFirstRun ) {
        VirtualProtect(lpbFirstRun, sizeof(BOOL), flNewProtect, &flOldProtect);
        *lpbFirstRun = FALSE;
        VirtualProtect(lpbFirstRun, sizeof(BOOL), flOldProtect, &flNewProtect);
        trace(_T("Patched boolean value #1: %p = %s"), lpbFirstRun, *lpbFirstRun ? L"TRUE" : L"FALSE");
        result = TRUE;
    }
    if ( !*lpbIsCPUSupportedResult ) {
        VirtualProtect(lpbIsCPUSupportedResult, sizeof(BOOL), flNewProtect, &flOldProtect);
        *lpbIsCPUSupportedResult = TRUE;
        VirtualProtect(lpbIsCPUSupportedResult, sizeof(BOOL), flOldProtect, &flNewProtect);
        trace(_T("Patched boolean value #2: %p = %s"), lpbIsCPUSupportedResult, *lpbIsCPUSupportedResult ? L"TRUE" : L"FALSE");
        result = TRUE;
    }
    if ( result )
        trace(_T("Successfully patched WUA module!"));

    return result;
}
