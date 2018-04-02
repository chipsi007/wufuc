#include "stdafx.h"
#include "versionhelper.h"

int ver_compare_product_version(VS_FIXEDFILEINFO *pffi, WORD wMajor, WORD wMinor, WORD wBuild, WORD wRev)
{
        if ( HIWORD(pffi->dwProductVersionMS) < wMajor ) return -1;
        if ( HIWORD(pffi->dwProductVersionMS) > wMajor ) return 1;
        if ( LOWORD(pffi->dwProductVersionMS) < wMinor ) return -1;
        if ( LOWORD(pffi->dwProductVersionMS) > wMinor ) return 1;
        if ( HIWORD(pffi->dwProductVersionLS) < wBuild ) return -1;
        if ( HIWORD(pffi->dwProductVersionLS) > wBuild ) return 1;
        if ( LOWORD(pffi->dwProductVersionLS) < wRev ) return -1;
        if ( LOWORD(pffi->dwProductVersionLS) > wRev ) return 1;
        return 0;
}

bool ver_verify_version_info(WORD wMajorVersion, WORD wMinorVersion, WORD wServicePackMajor)
{
        DWORDLONG dwlConditionMask = 0;
        OSVERSIONINFOEXW osvi = { sizeof osvi };

        VER_SET_CONDITION(dwlConditionMask, VER_MAJORVERSION, VER_EQUAL);
        VER_SET_CONDITION(dwlConditionMask, VER_MINORVERSION, VER_EQUAL);
        VER_SET_CONDITION(dwlConditionMask, VER_SERVICEPACKMAJOR, VER_GREATER_EQUAL);

        osvi.dwMajorVersion = wMajorVersion;
        osvi.dwMinorVersion = wMinorVersion;
        osvi.wServicePackMajor = wServicePackMajor;

        return VerifyVersionInfoW(&osvi,
                VER_MAJORVERSION | VER_MINORVERSION | VER_SERVICEPACKMAJOR,
                dwlConditionMask) != FALSE;
}
