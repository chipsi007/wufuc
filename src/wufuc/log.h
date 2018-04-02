#pragma once

void log_debug_(const wchar_t *const format, ...);
void log_trace_(const wchar_t *const format, ...);
void log_close(void);

#define log_debug(format, ...) log_debug_(__FUNCTIONW__ L"(" _CRT_WIDE(_CRT_STRINGIZE(__LINE__)) L"): [DEBUG] " format L"\r\n", ##__VA_ARGS__)
#define log_info(format, ...) log_trace_(__FUNCTIONW__ L"(" _CRT_WIDE(_CRT_STRINGIZE(__LINE__)) L"): [INFO] " format L"\r\n", ##__VA_ARGS__)
#define log_warning(format, ...) log_trace_(__FUNCTIONW__ L"(" _CRT_WIDE(_CRT_STRINGIZE(__LINE__)) L"): [WARNING] " format L"\r\n", ##__VA_ARGS__)
#define log_error(format, ...) log_trace_(__FUNCTIONW__ L"(" _CRT_WIDE(_CRT_STRINGIZE(__LINE__)) L"): [ERROR] " format L"\r\n", ##__VA_ARGS__)
