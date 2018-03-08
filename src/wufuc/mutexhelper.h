#pragma once

HANDLE mutex_create_new(bool InitialOwner, const wchar_t *MutexName);
HANDLE mutex_create_new_fmt(bool InitialOwner, const wchar_t *const NameFormat, ...);
