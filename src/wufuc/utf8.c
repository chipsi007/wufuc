#include "stdafx.h"
#include "utf8.h"

DWORD UTF8WriteFile(HANDLE hFile, LPCWSTR lpWideCharStr)
{
        int cchWideChar;
        int size;
        char *buffer;
        DWORD NumberOfBytesWritten = 0;

        cchWideChar = lstrlenW(lpWideCharStr);

        size = WideCharToMultiByte(CP_UTF8, 0, lpWideCharStr, cchWideChar, NULL, 0, NULL, NULL);
        if ( !size )
                return 0;

        buffer = malloc(size);
        if ( !buffer )
                return 0;

        if ( WideCharToMultiByte(CP_UTF8, 0, lpWideCharStr, cchWideChar, buffer, size, NULL, NULL) )
                WriteFile(hFile, buffer, size, &NumberOfBytesWritten, NULL);

        free(buffer);
        return NumberOfBytesWritten;
}
