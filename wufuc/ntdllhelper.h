#ifndef NTDLLHELPER_H_INCLUDED
#define NTDLLHELPER_H_INCLUDED
#pragma once

#define WIN32_NO_STATUS
#include <windows.h>
#undef WIN32_NO_STATUS

#include <winternl.h>

typedef enum tagKEY_INFORMATION_CLASS {
    KeyBasicInformation = 0,
    KeyNodeInformation = 1,
    KeyFullInformation = 2,
    KeyNameInformation = 3,
    KeyCachedInformation = 4,
    KeyFlagsInformation = 5,
    KeyVirtualizationInformation = 6,
    KeyHandleTagsInformation = 7,
    MaxKeyInfoClass = 8
} KEY_INFORMATION_CLASS;

typedef struct tagKEY_NAME_INFORMATION {
    ULONG NameLength;
    WCHAR Name[1];
} KEY_NAME_INFORMATION, *PKEY_NAME_INFORMATION;

typedef NTSTATUS(NTAPI *NTQUERYKEY)(HANDLE, KEY_INFORMATION_CLASS, PVOID, ULONG, PULONG);

BOOL TryNtQueryKey(HANDLE KeyHandle, KEY_INFORMATION_CLASS KeyInformationClass, PVOID KeyInformation, ULONG Length, PULONG ResultLength, PNTSTATUS status);
#endif
