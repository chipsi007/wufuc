#pragma once

BOOL logging_init(void);
VOID trace_(LPCWSTR format, ...);
BOOL logging_free(void);

#define STRINGIZE_(x) #x
#define STRINGIZE(x) STRINGIZE_(x)

#define STRINGIZEW_(x) L#x
#define STRINGIZEW(x) STRINGIZEW_(x)
#define __LINEWSTR__ STRINGIZEW(__LINE__)
#define trace(format, ...) trace_(__FILEW__ L":" __LINEWSTR__ L"(" __FUNCTIONW__ L"): " format L"\n", ##__VA_ARGS__)
