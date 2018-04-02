#pragma once

#ifndef BUILD_COMMIT_VERSION
#define BUILD_COMMIT_VERSION 1.0.1.0
#endif

#ifndef BUILD_VERSION_COMMA
#define BUILD_VERSION_COMMA 1,0,1,0
#endif

#define S_(x) #x
#define S(x) S_(x)

#ifdef X64
#define FILENAME "wufuc64.dll"
#elif defined(X86)
#define FILENAME "wufuc32.dll"
#endif
