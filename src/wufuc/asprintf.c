#include "stdafx.h"
#include "asprintf.h"
#include <locale.h>

int asprintf(char **strp, char const *const fmt, ...)
{
        va_list ap;
        int result;

        va_start(ap, fmt);
        result = vasprintf(strp, fmt, ap);
        va_end(ap);

        return result;
}

int vasprintf(char **strp, char const *const fmt, va_list ap)
{
        return vasprintf_l(strp, fmt, _get_current_locale(), ap);
}

int asprintf_l(char **strp, char const *const fmt, _locale_t locale, ...)
{
        va_list ap;
        int result;

        va_start(ap, locale);
        result = vasprintf_l(strp, fmt, locale, ap);
        va_end(ap);

        return result;
}

int vasprintf_l(char **strp, char const *const fmt, _locale_t locale, va_list ap)
{
        va_list copy;
        int result;
        char *str;
        int ret;

        va_copy(copy, ap);
        result = _vscprintf(fmt, copy);
        va_end(copy);

        if ( result == -1 )
                return result;

        str = calloc(result + 1, sizeof *str);
        if ( !str )
                return -1;

        va_copy(copy, ap);
        ret = _vsprintf_s_l(str, result + 1, fmt, locale, copy);
        va_end(copy);

        if ( ret == -1 ) {
                free(str);
                return -1;
        }
        *strp = str;
        return result;
}

int aswprintf(wchar_t **strp, wchar_t const *const fmt, ...)
{
        va_list ap;
        int result;

        va_start(ap, fmt);
        result = vaswprintf(strp, fmt, ap);
        va_end(ap);

        return result;
}

int vaswprintf(wchar_t **strp, wchar_t const *const fmt, va_list ap)
{
        return vaswprintf_l(strp, fmt, _get_current_locale(), ap);
}

int aswprintf_l(wchar_t **strp, wchar_t const *const fmt, _locale_t locale, ...)
{
        va_list ap;
        int result;

        va_start(ap, locale);
        result = vaswprintf_l(strp, fmt, locale, ap);
        va_end(ap);

        return result;
}

int vaswprintf_l(wchar_t **strp, wchar_t const *const fmt, _locale_t locale, va_list ap)
{
        va_list copy;
        int result;
        wchar_t *str;
        int ret;

        va_copy(copy, ap);
        result = _vscwprintf_l(fmt, locale, copy);
        va_end(copy);

        if ( result == -1 )
                return result;

        str = calloc(result + 1, sizeof *str);
        if ( !str )
                return -1;

        va_copy(copy, ap);
        ret = _vswprintf_s_l(str, result + 1, fmt, locale, copy);
        va_end(copy);

        if ( ret == -1 ) {
                free(str);
                return -1;
        }
        *strp = str;
        return result;
}
