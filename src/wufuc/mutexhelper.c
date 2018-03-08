#include "stdafx.h"
#include "mutexhelper.h"

#include <sddl.h>

HANDLE mutex_create_new(bool InitialOwner, const wchar_t *MutexName)
{
        HANDLE hMutex;

        hMutex = CreateMutexW(NULL, InitialOwner, MutexName);
        if ( hMutex ) {
                if ( GetLastError() == ERROR_ALREADY_EXISTS ) {
                        CloseHandle(hMutex);
                        return NULL;
                }
                return hMutex;
        }
        return NULL;
}

HANDLE mutex_create_new_fmt(bool InitialOwner, const wchar_t *const NameFormat, ...)
{
        HANDLE result = NULL;
        va_list ap;
        wchar_t *buffer;
        int ret;

        va_start(ap, NameFormat);
        ret = _vscwprintf(NameFormat, ap) + 1;
        va_end(ap);
        buffer = calloc(ret, sizeof *buffer);
        if ( buffer ) {
                va_start(ap, NameFormat);
                ret = vswprintf_s(buffer, ret, NameFormat, ap);
                va_end(ap);
                if (ret != -1) 
                        result = mutex_create_new(InitialOwner, buffer);
                free(buffer);
        }
        return result;
}
