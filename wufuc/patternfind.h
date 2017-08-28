#ifndef PATTERNFIND_H
#define PATTERNFIND_H
#pragma once

#include <Windows.h>

typedef struct _PATTERNBYTE {
    struct _PATTERNNIBBLE {
        unsigned char data;
        BOOL wildcard;
    } nibble[2];
} PATTERNBYTE, *PPATTERNBYTE, *LPPATTERNBYTE;

unsigned char *patternfind(unsigned char *data, size_t datasize, size_t startindex, const char *pattern);
unsigned char *patternsnr(unsigned char *data, size_t datasize, size_t startindex, const char *searchpattern, const char *replacepattern);
#endif
