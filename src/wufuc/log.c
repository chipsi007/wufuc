#include "stdafx.h"
#include "log.h"

#include <ShlObj.h>

static HANDLE m_hFile = INVALID_HANDLE_VALUE;
static DWORD m_dwProcessId;
static wchar_t m_szExeFilePath[MAX_PATH];
static wchar_t *m_pszExeName;

BOOL CALLBACK init_once_callback(
        PINIT_ONCE InitOnce,
        PVOID *Parameter,
        PVOID *lpContext)
{
        BOOL result;
        HRESULT hr;
        wchar_t *pszPath;
        wchar_t szFilePath[MAX_PATH];
        int ret;

        m_dwProcessId = GetCurrentProcessId();
        if ( !GetModuleFileNameW(NULL, m_szExeFilePath, _countof(m_szExeFilePath)) ) {
                log_debug(L"GetModuleFileNameW failed! (GLE=%lu)", GetLastError());
                return FALSE;
        }
        m_pszExeName = PathFindFileNameW(m_szExeFilePath);

        hr = SHGetKnownFolderPath(&FOLDERID_ProgramData, 0, NULL, &pszPath);
        if ( hr != S_OK ) {
                log_debug(L"SHGetKnownFolderPath failed! (HRESULT=0x%08X)", hr);
                return FALSE;
        }
        ret = wcscpy_s(szFilePath, _countof(szFilePath), pszPath);
        CoTaskMemFree(pszPath);
        if ( ret ) {
                log_debug(L"wcscpy_s failed! (Return value=%d)", ret);
                return FALSE;
        }
        if ( !PathAppendW(szFilePath, L"wufuc") ) {
append_fail:
                log_debug(L"PathAppendW failed!");
                return FALSE;
        }
        if ( !CreateDirectoryW(szFilePath, NULL)
                && GetLastError() != ERROR_ALREADY_EXISTS ) {

                log_debug(L"CreateDirectoryW failed! (GLE=%lu)", GetLastError());
                return FALSE;
        }
        if ( !PathAppendW(szFilePath, L"wufuc.1.log") )
                goto append_fail;

        m_hFile = CreateFileW(szFilePath,
                FILE_APPEND_DATA,
                FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                NULL,
                OPEN_ALWAYS,
                FILE_ATTRIBUTE_NORMAL,
                NULL);

        result = m_hFile != INVALID_HANDLE_VALUE;
        if ( !result )
                log_debug(L"CreateFileW failed! (GLE=%lu)", GetLastError());
        return result;
}

void log_debug_(const wchar_t *const format, ...)
{
        va_list ap;
        wchar_t *buf;
        int ret;
        int count;

        va_start(ap, format);
        count = _vscwprintf(format, ap);
        va_end(ap);
        if ( count == -1 ) return;

        buf = calloc(count + 1, sizeof *buf);
        if ( !buf ) return;

        va_start(ap, format);
        ret = vswprintf_s(buf, count + 1, format, ap);
        va_end(ap);
        if ( ret != -1 )
                OutputDebugStringW(buf);
        free(buf);
}

void log_trace_(const wchar_t *const format, ...)
{
        static INIT_ONCE InitOnce = INIT_ONCE_STATIC_INIT;
        BOOL bStatus;
        wchar_t datebuf[9];
        wchar_t timebuf[9];
        va_list ap;
        const wchar_t fmt[] = L"%ls %ls [%ls:%lu] %ls";
        int count;
        wchar_t *buf1;
        int ret;
        wchar_t *buf2;
        int size;
        char *buf3;
        DWORD written;

        bStatus = InitOnceExecuteOnce(&InitOnce, init_once_callback, NULL, NULL);

        if ( _wstrdate_s(datebuf, _countof(datebuf))
                || _wstrtime_s(timebuf, _countof(timebuf)) )
                return;

        va_start(ap, format);
        ret = _vscwprintf(format, ap);
        va_end(ap);
        if ( ret == -1 ) return;
        count = ret + 1;

        buf1 = calloc(count, sizeof *buf1);
        if ( !buf1 ) return;

        va_start(ap, format);
        ret = vswprintf_s(buf1, count, format, ap);
        va_end(ap);
        if ( ret == -1 ) goto free_buf1;

        ret = _scwprintf(fmt, datebuf, timebuf, m_pszExeName, m_dwProcessId, buf1);
        if ( ret == -1 ) goto free_buf1;
        count = ret + 1;

        buf2 = calloc(count, sizeof *buf2);
        if ( !buf2 ) goto free_buf1;

        ret = swprintf_s(buf2, count, fmt, datebuf, timebuf, m_pszExeName, m_dwProcessId, buf1);
        if ( ret == -1 ) goto free_buf2;

        if ( bStatus ) {
                size = WideCharToMultiByte(CP_UTF8, 0, buf2, ret, NULL, 0, NULL, NULL);
                if ( !size ) goto fallback;

                buf3 = malloc(size);
                if ( !buf3 ) goto fallback;

                ret = WideCharToMultiByte(CP_UTF8, 0, buf2, ret, buf3, size, NULL, NULL)
                        && WriteFile(m_hFile, buf3, size, &written, NULL);
                free(buf3);
                if ( !ret ) goto fallback;
        } else {
fallback:
                OutputDebugStringW(buf2);
        }
free_buf2:
        free(buf2);
free_buf1:
        free(buf1);
}

void log_close(void)
{
        if ( m_hFile != INVALID_HANDLE_VALUE )
                CloseHandle(m_hFile);
}
