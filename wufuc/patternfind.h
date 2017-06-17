#pragma once

typedef struct _PATTERNBYTE {
    struct _PATTERNNIBBLE {
        BYTE data;
        BOOL wildcard;
    } nibble[2];
} PATTERNBYTE, *PPATTERNBYTE, *LPPATTERNBYTE;

int hexchtoint(CHAR ch);
SIZE_T formathexpattern(LPCSTR patterntext, LPSTR formattext, SIZE_T formattextsize);
BOOL patterntransform(LPCSTR patterntext, LPPATTERNBYTE pattern, SIZE_T *patternsize);
SIZE_T patternfind(LPCBYTE data, SIZE_T datasize, SIZE_T startindex, LPCSTR pattern);
VOID patternwritebyte(LPBYTE byte, LPPATTERNBYTE pbyte);
VOID patternwrite(LPBYTE data, SIZE_T datasize, LPCSTR pattern);
SIZE_T patternsnr(LPBYTE data, SIZE_T datasize, SIZE_T startindex, LPCSTR searchpattern, LPCSTR replacepattern);
