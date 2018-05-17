#pragma once

HANDLE CreateNewMutex(LPSECURITY_ATTRIBUTES lpMutexAttributes, BOOL bInitialOwner, LPCWSTR lpName);

HANDLE CreateEventWithStringSecurityDescriptor(
        LPCWSTR lpStringSecurityDescriptor,
        BOOL bManualReset,
        BOOL bInitialState,
        LPCWSTR lpName);

BOOL VerifyVersionInfoHelper(WORD wMajorVersion, WORD wMinorVersion, WORD wServicePackMajor);
