#include <Windows.h>

#include "logging.h"
#include "iathook.h"

VOID iat_hook(HMODULE hModule, LPCSTR lpFuncName, LPVOID *lpOldAddress, LPVOID lpNewAddress) {
    LPVOID *lpAddress = iat_find(hModule, lpFuncName);
    if (!lpAddress || *lpAddress == lpNewAddress)
        return;

    if (!hModule)
        hModule = GetModuleHandle(NULL);

    trace(L"Modified import address: hmod=%p, import=%S, old addr=%p, new addr=%p", hModule, lpFuncName, *lpAddress, lpNewAddress);

    DWORD flOldProtect;
    DWORD flNewProtect = PAGE_READWRITE;
    VirtualProtect(lpAddress, sizeof(LPVOID), flNewProtect, &flOldProtect);
    if (lpOldAddress)
        *lpOldAddress = *lpAddress;
    *lpAddress = lpNewAddress;
    VirtualProtect(lpAddress, sizeof(LPVOID), flOldProtect, &flNewProtect);
}

static LPVOID *iat_find(HMODULE hModule, LPCSTR lpFunctionName) {
    uintptr_t hm = (uintptr_t)hModule;

    for (PIMAGE_IMPORT_DESCRIPTOR iid = (PIMAGE_IMPORT_DESCRIPTOR)(hm + ((PIMAGE_NT_HEADERS)(hm + ((PIMAGE_DOS_HEADER)hm)->e_lfanew))
        ->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress); iid->Name; iid++) {

        LPVOID *pp;
        for (SIZE_T i = 0; *(pp = i + (LPVOID *)(hm + iid->FirstThunk)); i++) {
            LPSTR fn = (LPSTR)(hm + *(i + (SIZE_T *)(hm + iid->OriginalFirstThunk)) + 2);
            if (!((uintptr_t)fn & IMAGE_ORDINAL_FLAG) && !_stricmp(lpFunctionName, fn))
                return pp;
        }
    }
    return NULL;
}
