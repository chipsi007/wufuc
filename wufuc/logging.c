#include <Windows.h>
#include <stdio.h>
#include <time.h>
#include <tchar.h>
#include <Psapi.h>

#include "helpers.h"
#include "logging.h"

static FILE *fp;
static BOOL logging_enabled = FALSE;

BOOL logging_init(void) {
    if (fp)
        return TRUE;

    WCHAR filename[MAX_PATH];
    GetModuleFileNameW(HINST_THISCOMPONENT, filename, _countof(filename));
    WCHAR drive[_MAX_DRIVE], dir[_MAX_DIR], fname[_MAX_FNAME];
    _wsplitpath_s(filename, drive, _countof(drive), dir, _countof(dir), fname, _countof(fname), NULL, 0);

    WCHAR basename[MAX_PATH];
    GetModuleBaseNameW(GetCurrentProcess(), NULL, basename, _countof(basename));
    wcscat_s(fname, _countof(fname), L".");
    wcscat_s(fname, _countof(fname), basename);
    _wmakepath_s(filename, _countof(filename), drive, dir, fname, L".log");

    HANDLE hFile = CreateFileW(filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    LARGE_INTEGER size;
    GetFileSizeEx(hFile, &size);
    CloseHandle(hFile);
    fp = _wfsopen(filename, size.QuadPart < (1 << 20) ? L"at" : L"wt", _SH_DENYWR);
    return (fp != NULL);
}

VOID trace_(LPCWSTR format, ...) {
    if (logging_init()) {
        WCHAR datebuf[9], timebuf[9];
        _wstrdate_s(datebuf, _countof(datebuf));
        _wstrtime_s(timebuf, _countof(timebuf));
        fwprintf_s(fp, L"%s %s [%d] ", datebuf, timebuf, GetCurrentProcessId());

        va_list argptr;
        va_start(argptr, format);
        vfwprintf_s(fp, format, argptr);
        va_end(argptr);
        fflush(fp);
    }
}

BOOL logging_free(void) {
    return fp && !fclose(fp);
}
