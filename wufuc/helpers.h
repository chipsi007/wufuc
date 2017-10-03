#pragma once

#include <stdbool.h>

#include <phnt_windows.h>

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
);

wchar_t *find_fname(wchar_t *pPath);
BOOL file_exists(const wchar_t *path);
int compare_versions(
        WORD wMajorA, WORD wMinorA, WORD wBuildA, WORD wRevisionA,
        WORD wMajorB, WORD wMinorB, WORD wBuildB, WORD wRevisionB);

