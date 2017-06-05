#pragma once

#ifdef UNICODE
#define CommandLineToArgv  CommandLineToArgvW
#else
#define CommandLineToArgv  CommandLineToArgvA
#endif // !UNICODE
