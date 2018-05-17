#include "stdafx.h"
#include "helpers.h"
#include <sddl.h>

HANDLE CreateEventWithStringSecurityDescriptor(
        LPCWSTR lpStringSecurityDescriptor,
        BOOL bManualReset,
        BOOL bInitialState,
        LPCWSTR lpName)
{
        SECURITY_ATTRIBUTES sa = { sizeof sa };

        if ( ConvertStringSecurityDescriptorToSecurityDescriptorW(
                lpStringSecurityDescriptor,
                SDDL_REVISION_1,
                &sa.lpSecurityDescriptor,
                NULL) ) {
                return CreateEventW(&sa, bManualReset, bInitialState, lpName);
        }
        return NULL;
}

HANDLE CreateNewMutex(LPSECURITY_ATTRIBUTES lpMutexAttributes, BOOL bInitialOwner, LPCWSTR lpName)
{
        HANDLE result;

        result = CreateMutexW(lpMutexAttributes, bInitialOwner, lpName);
        if ( result ) {
                if ( GetLastError() == ERROR_ALREADY_EXISTS ) {
                        CloseHandle(result);
                        return NULL;
                }
                return result;
        }
        return NULL;
}

BOOL VerifyVersionInfoHelper(WORD wMajorVersion, WORD wMinorVersion, WORD wServicePackMajor)
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
                dwlConditionMask);
}
