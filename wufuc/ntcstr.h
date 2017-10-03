#pragma once

#include <phnt_windows.h>

#define _countof(_Array) (sizeof(_Array) / sizeof(_Array[0]))
#define _max(a,b) (((a) > (b)) ? (a) : (b))
#define _min(a,b) (((a) < (b)) ? (a) : (b))

#define _MAX_PATH   260 // max. length of full pathname
#define _MAX_DRIVE  3   // max. length of drive component
#define _MAX_DIR    256 // max. length of path component
#define _MAX_FNAME  256 // max. length of file name component
#define _MAX_EXT    256 // max. length of extension component

typedef int(__cdecl *LPFN__WCSICMP_NTDLL)(const wchar_t *string1, const wchar_t *string2);

int _wcsicmp_Ntdll(
        const wchar_t *string1,
        const wchar_t *string2
);