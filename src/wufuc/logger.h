#pragma once

void log_debug_(const wchar_t *const format, ...);
void log_trace_(const wchar_t *const format, ...);
void log_close(void);

#define log_debug(format, ...) log_debug_(L"[DEBUG] " __FUNCTIONW__ L"(" _CRT_WIDE(_CRT_STRINGIZE(__LINE__)) L"): " format L"\r\n", ##__VA_ARGS__)
#define log_info(format, ...) log_trace_(L"[INFO] " __FUNCTIONW__ L"(" _CRT_WIDE(_CRT_STRINGIZE(__LINE__)) L"): " format L"\r\n", ##__VA_ARGS__)
#define log_warning(format, ...) log_trace_(L"[WARNING] " __FUNCTIONW__ L"(" _CRT_WIDE(_CRT_STRINGIZE(__LINE__)) L"): " format L"\r\n", ##__VA_ARGS__)
#define log_error(format, ...) log_trace_(L"[ERROR] " __FUNCTIONW__ L"(" _CRT_WIDE(_CRT_STRINGIZE(__LINE__)) L"): " format L"\r\n", ##__VA_ARGS__)
#define log_gle(format, ...) log_trace_(L"[ERROR] " __FUNCTIONW__ L"(" _CRT_WIDE(_CRT_STRINGIZE(__LINE__)) L"): " format L" (LastError=%lu)\r\n", ##__VA_ARGS__, GetLastError())
