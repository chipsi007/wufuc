#ifndef PATTERNFIND_H_INCLUDED
#define PATTERNFIND_H_INCLUDED
#pragma once

#include <Windows.h>

typedef struct tagPATTERNBYTE {
    struct tagPATTERNNIBBLE {
        unsigned char data;
        BOOL wildcard;
    } nibble[2];
} PATTERNBYTE, *PPATTERNBYTE;

unsigned char *patternfind(unsigned char *data, size_t datasize, size_t startindex, const char *pattern);
unsigned char *patternsnr(unsigned char *data, size_t datasize, size_t startindex, const char *searchpattern, const char *replacepattern);
#endif
