#include "stdafx.h"
#include "log.h"

#include <ShlObj.h>

HANDLE m_hFile = INVALID_HANDLE_VALUE;

BOOL CALLBACK init_file_handle(
        PINIT_ONCE pInitOnce,
        ParamData *pParam,
        PVOID *ppContext)
{
        BOOL result = FALSE;
        HANDLE hFile;
        HRESULT hr;
        wchar_t *pszPath;
        wchar_t szFilePath[MAX_PATH];
        int ret;

        pParam->dwProcessId = GetCurrentProcessId();
        if ( !GetModuleFileNameW(NULL, pParam->szExeFilePath, _countof(pParam->szExeFilePath)) ) {
                log_debug(L"GetModuleFileNameW failed! (GLE=%lu)", GetLastError());
                return result;
        }
        pParam->pszExeName = PathFindFileNameW(pParam->szExeFilePath);

        hr = SHGetKnownFolderPath(&FOLDERID_ProgramData, 0, NULL, &pszPath);
        if ( hr != S_OK ) {
                log_debug(L"SHGetKnownFolderPath failed! (HRESULT=0x%08X)", hr);
                return result;
        }
        ret = wcscpy_s(szFilePath, _countof(szFilePath), pszPath);
        CoTaskMemFree(pszPath);
        if ( ret ) {
                log_debug(L"wcscpy_s failed! (Return value=%d)", ret);
                return result;
        }

        if ( !PathAppendW(szFilePath, L"wufuc") ) {
append_fail:
                log_debug(L"PathAppendW failed!");
                return result;
        }
        if ( !CreateDirectoryW(szFilePath, NULL)
                && GetLastError() != ERROR_ALREADY_EXISTS ) {

                log_debug(L"CreateDirectoryW failed! (GLE=%lu)", GetLastError());
                return result;
        }
        if ( !PathAppendW(szFilePath, L"wufuc.log") )
                goto append_fail;

        hFile = CreateFileW(szFilePath,
                FILE_APPEND_DATA,
                FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                NULL,
                OPEN_ALWAYS,
                FILE_ATTRIBUTE_NORMAL,
                NULL);

        if ( hFile != INVALID_HANDLE_VALUE ) {
                *ppContext = (PVOID)hFile;
                result = TRUE;
        } else {
                log_debug(L"CreateFileW failed! (GLE=%lu)", GetLastError());
        }
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
        static ParamData data;
        BOOL bStatus;
        errno_t e;
        wchar_t datebuf[9];
        wchar_t timebuf[9];
        va_list ap;
        const wchar_t fmt[] = L"%ls %ls [%ls:%lu] %ls";
        int count;
        wchar_t *buf1;
        int ret;
        wchar_t *buf2;
        DWORD written;

        bStatus = InitOnceExecuteOnce(&InitOnce,
                (PINIT_ONCE_FN)init_file_handle,
                &data,
                &(LPVOID)m_hFile);

        e = _wstrdate_s(datebuf, _countof(datebuf));
        if ( e ) return;
        e = _wstrtime_s(timebuf, _countof(timebuf));
        if ( e ) return;

        va_start(ap, format);
        count = _vscwprintf(format, ap);
        va_end(ap);
        if ( count == -1 ) return;

        buf1 = calloc(count + 1, sizeof *buf1);
        if ( !buf1 ) return;

        va_start(ap, format);
        ret = vswprintf_s(buf1, count + 1, format, ap);
        va_end(ap);
        if ( ret == -1 ) goto free_buf1;

        count = _scwprintf(fmt, datebuf, timebuf, data.pszExeName, data.dwProcessId, buf1);
        if ( count == -1 ) goto free_buf1;

        buf2 = calloc(count + 1, sizeof *buf2);
        if ( !buf2 ) goto free_buf1;

        ret = swprintf_s(buf2, count + 1, fmt, datebuf, timebuf, data.pszExeName, data.dwProcessId, buf1);
        if ( ret == -1 ) goto free_buf2;

        if ( !bStatus || !WriteFile(m_hFile, buf2, count * (sizeof *buf2), &written, NULL) )
                OutputDebugStringW(buf2);
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
