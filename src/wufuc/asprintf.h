#pragma once

int asprintf(char **strp, char const *const fmt, ...);

int vasprintf(char **strp, char const *const fmt, va_list ap);

int asprintf_l(char **strp, char const *const fmt, _locale_t locale, ...);

int vasprintf_l(char **strp, char const *const fmt, _locale_t locale, va_list ap);

int aswprintf(wchar_t **strp, wchar_t const *const fmt, ...);

int vaswprintf(wchar_t **strp, wchar_t const *const fmt, va_list ap);

int aswprintf_l(wchar_t **strp, wchar_t const *const fmt, _locale_t locale, ...);

int vaswprintf_l(wchar_t **strp, wchar_t const *const fmt, _locale_t locale, va_list ap);
