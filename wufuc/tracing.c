#include "tracing.h"

#include <stdio.h>
#include <time.h>

#include <Windows.h>
#include <tchar.h>

static FILE *m_pStream;
static HANDLE m_hMutex;
static BOOL m_bDeinitializing;

BOOL InitTracing(void) {
    if ( m_bDeinitializing )
        return FALSE;

    if ( !m_hMutex )
        m_hMutex = CreateMutex(NULL, FALSE, _T("Global\\wufuc_TracingMutex"));

    if ( m_hMutex && !m_pStream ) {
        TCHAR path[MAX_PATH];
        GetModuleFileName(HINST_THISCOMPONENT, path, _countof(path));

        TCHAR drive[_MAX_DRIVE], dir[_MAX_DIR], fname[_MAX_FNAME];
        _tsplitpath_s(path, drive, _countof(drive), dir, _countof(dir), fname, _countof(fname), NULL, 0);
        _tmakepath_s(path, _countof(path), drive, dir, fname, _T(".log"));

        m_pStream = _tfsopen(path, _T("at"), _SH_DENYNO);
    }
    return m_pStream && m_hMutex;
}

DWORD WaitForTracingMutex(void) {
    return WaitForSingleObject(m_hMutex, INFINITE);
}

BOOL ReleaseTracingMutex(void) {
    return ReleaseMutex(m_hMutex);
}

void trace_(LPCTSTR format, ...) {
    if ( InitTracing() ) {
        TCHAR datebuf[9], timebuf[9];
        _tstrdate_s(datebuf, _countof(datebuf));
        _tstrtime_s(timebuf, _countof(timebuf));
        if ( !WaitForTracingMutex() ) {
            _ftprintf_s(m_pStream, _T("%s %s [PID: %d TID: %d] "), datebuf, timebuf, GetCurrentProcessId(), GetCurrentThreadId());

            va_list argptr;
            va_start(argptr, format);
            _vftprintf_s(m_pStream, format, argptr);
            va_end(argptr);
            fflush(m_pStream);
        }
        ReleaseTracingMutex();
    }
}

BOOL DeinitTracing(void) {
    m_bDeinitializing = TRUE;

    BOOL result = TRUE;
    if ( m_hMutex ) {
        result = CloseHandle(m_hMutex);
        m_hMutex = NULL;
    }
    if ( m_pStream ) {
        result = result && !fclose(m_pStream);
        m_pStream = NULL;
    }
    return result;
}
