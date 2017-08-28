#include "logging.h"

#include <stdio.h>
#include <time.h>

#include <Windows.h>
#include <tchar.h>
#include <Psapi.h>

static FILE *fp;
static BOOL logging_enabled = FALSE;

BOOL InitLogging(void) {
    if ( fp )
        return TRUE;

    TCHAR filename[MAX_PATH];
    GetModuleFileName(HINST_THISCOMPONENT, filename, _countof(filename));

    TCHAR drive[_MAX_DRIVE], dir[_MAX_DIR], fname[_MAX_FNAME];
    _tsplitpath_s(filename, drive, _countof(drive), dir, _countof(dir), fname, _countof(fname), NULL, 0);

    TCHAR basename[MAX_PATH];
    GetModuleBaseNameW(GetCurrentProcess(), NULL, basename, _countof(basename));
    _tcscat_s(fname, _countof(fname), _T("."));
    _tcscat_s(fname, _countof(fname), basename);
    _tmakepath_s(filename, _countof(filename), drive, dir, fname, _T(".log"));

    HANDLE hFile = CreateFile(filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    LARGE_INTEGER size;
    GetFileSizeEx(hFile, &size);
    CloseHandle(hFile);
    fp = _tfsopen(filename, size.QuadPart < (1 << 20) ? _T("at") : _T("wt"), _SH_DENYWR);
    return (fp != NULL);
}

void trace_(LPCTSTR format, ...) {
    static DWORD dwProcessId = 0;

    if ( InitLogging() ) {
        if ( !dwProcessId )
            dwProcessId = GetCurrentProcessId();

        TCHAR datebuf[9], timebuf[9];
        _tstrdate_s(datebuf, _countof(datebuf));
        _tstrtime_s(timebuf, _countof(timebuf));
        _ftprintf_s(fp, _T("%s %s [%d] "), datebuf, timebuf, dwProcessId);

        va_list argptr;
        va_start(argptr, format);
        _vftprintf_s(fp, format, argptr);
        va_end(argptr);
        fflush(fp);
    }
}

BOOL FreeLogging(void) {
    return fp && !fclose(fp);
}
