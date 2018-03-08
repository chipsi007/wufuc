#include "stdafx.h"
#include "log.h"

void logp_debug_write(const wchar_t *const format, ...)
{
        DWORD pid;
        wchar_t exepath[MAX_PATH];
        wchar_t *exename;
        va_list ap;
        int count;
        wchar_t *buffer1;
        wchar_t *buffer2;
        wchar_t datebuf[9];
        wchar_t timebuf[9];
        const wchar_t fmt[] = L"%ls %ls [%ls:%lu] %ls";

        pid = GetCurrentProcessId();
        GetModuleFileNameW(NULL, exepath, _countof(exepath));
        exename = PathFindFileNameW(exepath);

        va_start(ap, format);
        count = _vscwprintf(format, ap) + 1;
        va_end(ap);
        buffer1 = calloc(count, sizeof *buffer1);
        va_start(ap, format);
        vswprintf_s(buffer1, count, format, ap);
        va_end(ap);

        _wstrdate_s(datebuf, _countof(datebuf));
        _wstrtime_s(timebuf, _countof(timebuf));
        count = _scwprintf(fmt, datebuf, timebuf, exename, pid, buffer1) + 1;

        buffer2 = calloc(count, sizeof *buffer2);
        swprintf_s(buffer2, count, fmt, datebuf, timebuf, exename, pid, buffer1);

        free(buffer1);
        OutputDebugStringW(buffer2);
        free(buffer2);
}
