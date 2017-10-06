#include "helpers.h"

#include <stdint.h>
#include <stdbool.h>

#include <phnt_windows.h>
#include <phnt.h>

bool verify_winver(
        DWORD dwMajorVersion,
        DWORD dwMinorVersion,
        DWORD dwBuildNumber,
        WORD wServicePackMajor,
        WORD wServicePackMinor,
        BYTE MajorVersionCondition,
        BYTE MinorVersionCondition,
        BYTE BuildNumberCondition,
        BYTE ServicePackMajorCondition,
        BYTE ServicePackMinorCondition
)
{
        RTL_OSVERSIONINFOEXW osvi = { 0 };
        osvi.dwOSVersionInfoSize = sizeof(RTL_OSVERSIONINFOEXW);

        osvi.dwMajorVersion = dwMajorVersion;
        osvi.dwMinorVersion = dwMinorVersion;
        osvi.dwBuildNumber = dwBuildNumber;
        osvi.wServicePackMajor = wServicePackMajor;
        osvi.wServicePackMinor = wServicePackMinor;

        ULONGLONG ConditionMask = 0;
        ULONG TypeMask = 0;
        if ( MajorVersionCondition ) {
                TypeMask |= VER_MAJORVERSION;
                VER_SET_CONDITION(ConditionMask, VER_MAJORVERSION, MajorVersionCondition);
        }
        if ( MinorVersionCondition ) {
                TypeMask |= VER_MINORVERSION;
                VER_SET_CONDITION(ConditionMask, VER_MINORVERSION, MinorVersionCondition);
        }
        if ( BuildNumberCondition ) {
                TypeMask |= VER_BUILDNUMBER;
                VER_SET_CONDITION(ConditionMask, VER_BUILDNUMBER, BuildNumberCondition);
        }
        if ( ServicePackMajorCondition ) {
                TypeMask |= VER_SERVICEPACKMAJOR;
                VER_SET_CONDITION(ConditionMask, VER_SERVICEPACKMAJOR, ServicePackMajorCondition);
        }
        if ( ServicePackMinorCondition ) {
                TypeMask |= VER_SERVICEPACKMINOR;
                VER_SET_CONDITION(ConditionMask, VER_SERVICEPACKMINOR, ServicePackMinorCondition);
        }
        return RtlVerifyVersionInfo(&osvi, TypeMask, ConditionMask) == STATUS_SUCCESS;
}

bool verify_win7(void)
{
        static bool a, b;
        if ( !a ) {
                b = verify_winver(6, 1, 0, 0, 0, VER_EQUAL, VER_EQUAL, 0, 0, 0);
                a = true;
        }
        return b;
}

bool verify_win81(void)
{
        static bool a, b;
        if ( !a ) {
                b = verify_winver(6, 3, 0, 0, 0, VER_EQUAL, VER_EQUAL, 0, 0, 0);
                a = true;
        }
        return b;
}

wchar_t *find_fname(wchar_t *pPath)
{
        wchar_t *pwc = wcsrchr(pPath, L'\\');
        if ( pwc && *(++pwc) )
                return pwc;

        return pPath;
}

bool file_exists(const wchar_t *path)
{
        return GetFileAttributesW(path) != INVALID_FILE_ATTRIBUTES;
}

int compare_versions(
        WORD wMajorA, WORD wMinorA, WORD wBuildA, WORD wRevisionA,
        WORD wMajorB, WORD wMinorB, WORD wBuildB, WORD wRevisionB
)
{
        if ( wMajorA < wMajorB ) return -1;
        if ( wMajorA > wMajorB ) return 1;
        if ( wMinorA < wMinorB ) return -1;
        if ( wMinorA > wMinorB ) return 1;
        if ( wBuildA < wBuildB ) return -1;
        if ( wBuildA > wBuildB ) return 1;
        if ( wRevisionA < wRevisionB ) return -1;
        if ( wRevisionA > wRevisionB ) return 1;
        return 0;
}
