#include "stdafx.h"
#include "logger.h"
#include "utf8.h"
#include <ShlObj.h>

static WCHAR s_szExeFilePath[MAX_PATH];
static LPWSTR s_pszExeFileName;
static HANDLE s_hFile = INVALID_HANDLE_VALUE;

static BOOL CALLBACK init_once_callback(
        PINIT_ONCE InitOnce,
        PVOID *Parameter,
        PVOID *lpContext)
{
        PWSTR pszPath;
        LPWSTR folder;
        int count;
        LPWSTR file;

        if ( !GetModuleFileNameW(NULL, s_szExeFilePath, _countof(s_szExeFilePath)) )
                return FALSE;

        s_pszExeFileName = PathFindFileNameW(s_szExeFilePath);

        if ( FAILED(SHGetKnownFolderPath(&FOLDERID_ProgramData, KF_FLAG_DEFAULT, NULL, &pszPath)) )
                return FALSE;

        count = aswprintf(&folder, L"%ls\\wufuc", pszPath);
        CoTaskMemFree(pszPath);

        if ( count == -1 )
                return FALSE;

        if ( (CreateDirectoryW(folder, NULL) || GetLastError() == ERROR_ALREADY_EXISTS)
                && aswprintf(&file, L"%ls\\wufuc.1.log", folder) != -1 ) {

                s_hFile = CreateFileW(file,
                        FILE_APPEND_DATA,
                        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                        NULL,
                        OPEN_ALWAYS,
                        FILE_ATTRIBUTE_NORMAL,
                        NULL);
                free(file);
        }
        free(folder);
        return s_hFile != INVALID_HANDLE_VALUE;
}

void log_debug_(const wchar_t *const format, ...)
{
        va_list ap;
        int count;
        wchar_t *buffer;

        va_start(ap, format);
        count = vaswprintf(&buffer, format, ap);
        va_end(ap);
        if ( count != -1 )
                OutputDebugStringW(buffer);
        free(buffer);
}

void log_trace_(const wchar_t *const format, ...)
{
        static INIT_ONCE InitOnce = INIT_ONCE_STATIC_INIT;
        BOOL status;
        wchar_t datebuf[9];
        wchar_t timebuf[9];
        va_list ap;
        int count;
        wchar_t *buf1;
        wchar_t *buf2;

        status = InitOnceExecuteOnce(&InitOnce, init_once_callback, NULL, NULL);

        if ( _wstrdate_s(datebuf, _countof(datebuf))
                || _wstrtime_s(timebuf, _countof(timebuf)) )
                return;

        va_start(ap, format);
        count = vaswprintf(&buf1, format, ap);
        va_end(ap);
        if ( count == -1 ) return;

        count = aswprintf(&buf2, L"%ls %ls [%ls:%lu] %ls", datebuf, timebuf, s_pszExeFileName, GetCurrentProcessId(), buf1);
        free(buf1);
        if ( count == -1 ) return;

        if ( !status || !UTF8WriteFile(s_hFile, buf2) )
                OutputDebugStringW(buf2);
        free(buf2);
}

void log_close(void)
{
        if ( s_hFile != INVALID_HANDLE_VALUE )
                CloseHandle(s_hFile);
}
