#pragma once

#ifndef BUILD_COMMIT_VERSION
#define BUILD_COMMIT_VERSION 1.0.1.0
#endif

#ifndef BUILD_VERSION_COMMA
#define BUILD_VERSION_COMMA 1,0,1,0
#endif

#define STRINGIZE_(x) #x
#define STRINGIZE(x) STRINGIZE_(x)

#ifdef _WIN64
#define FILENAME "wufuc64.dll"
#else
#define FILENAME "wufuc32.dll"
#endif
