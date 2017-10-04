#pragma once

#include <stddef.h>
#include <stdbool.h>

typedef struct tagPatternByte
{
        struct PatternNibble
        {
                unsigned char data;
                bool wildcard;
        } nibble[2];
} PatternByte;

//returns: pointer to data when found, NULL when not found
unsigned char *patternfind(
        unsigned char *data, //data
        size_t datasize, //size of data
        const char *pattern //pattern to search
);

//returns: pointer to data when found, NULL when not found
unsigned char *patternfind2(
        unsigned char *data, //data
        size_t datasize, //size of data
        unsigned char *pattern, //bytes to search
        size_t patternsize //size of bytes to search
);

//returns: nothing
void patternwrite(
        unsigned char *data, //data
        size_t datasize, //size of data
        const char *pattern //pattern to write
);

//returns: true on success, false on failure
bool patternsnr(
        unsigned char *data, //data
        size_t datasize, //size of data
        const char *searchpattern, //pattern to search
        const char *replacepattern //pattern to write
);

//returns: true on success, false on failure
bool patterntransform(
        const char *patterntext, //pattern string
        PatternByte *pattern, //pattern to feed to patternfind
        size_t patternsize //size of pattern
);

//returns: pointer to data when found, NULL when not found
unsigned char *patternfind3(
        unsigned char *data, //data
        size_t datasize, //size of data
        const PatternByte *pattern, //pattern to search
        size_t searchpatternsize //size of pattern to search
);
