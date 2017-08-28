#include "patternfind.h"

#include <Windows.h>

/* Ported to C from x64dbg's patternfind.cpp:
 *   https://github.com/x64dbg/x64dbg/blob/development/src/dbg/patternfind.cpp
 * x64dbg license (GPL-3.0):
 *   https://github.com/x64dbg/x64dbg/blob/development/LICENSE
 */

static int hexchtoint(char c) {
    int result = -1;
    if ( c >= '0' && c <= '9' )
        result = c - '0';
    else if ( c >= 'A' && c <= 'F' )
        result = c - 'A' + 10;
    else if ( c >= 'a' && c <= 'f' )
        result = c - 'a' + 10;

    return result;
}

static size_t formathexpattern(const char *patterntext, char *formattext, size_t formattextsize) {
    size_t len = strlen(patterntext);
    size_t result = 0;
    for ( size_t i = 0; i < len && (!formattext || result < formattextsize); i++ ) {
        if ( patterntext[i] == '?' || hexchtoint(patterntext[i]) != -1 ) {
            if ( formattext )
                formattext[result] = patterntext[i];

            result++;
        }
    }
    return result;
}

static BOOL patterntransform(const char *patterntext, LPPATTERNBYTE pattern, size_t *patternsize) {
    size_t cb = formathexpattern(patterntext, NULL, 0);
    if ( !cb || cb > *patternsize )
        return FALSE;

    char *formattext = calloc(cb, sizeof(char));
    cb = formathexpattern(patterntext, formattext, cb);

    if ( cb % 2 )
        formattext[cb++] = '?';

    formattext[cb] = '\0';

    for ( size_t i = 0, j = 0, k = 0; i < cb; i++, j ^= 1, k = (i - j) >> 1 ) {
        if ( formattext[i] == '?' )
            pattern[k].nibble[j].wildcard = TRUE;
        else {
            pattern[k].nibble[j].wildcard = FALSE;
            pattern[k].nibble[j].data = hexchtoint(formattext[i]) & 0xf;
        }
    }
    free(formattext);
    *patternsize = cb >> 1;
    return TRUE;
}

static void patternwritebyte(unsigned char *byte, LPPATTERNBYTE pbyte) {
    unsigned char n1 = (*byte >> 4) & 0xf;
    unsigned char n2 = *byte & 0xf;
    if ( !pbyte->nibble[0].wildcard )
        n1 = pbyte->nibble[0].data;

    if ( !pbyte->nibble[1].wildcard )
        n2 = pbyte->nibble[1].data;
    *byte = ((n1 << 4) & 0xf0) | (n2 & 0xf);
}

static BOOL patternwrite(unsigned char *data, size_t datasize, const char *pattern) {
    size_t writepatternsize = strlen(pattern);
    if ( writepatternsize > datasize )
        writepatternsize = datasize;

    BOOL result = FALSE;
    LPPATTERNBYTE writepattern = calloc(writepatternsize, sizeof(PATTERNBYTE));
    if ( patterntransform(pattern, writepattern, &writepatternsize) ) {
        DWORD flNewProtect = PAGE_READWRITE;
        DWORD flOldProtect;

        if ( result = VirtualProtect(data, writepatternsize, flNewProtect, &flOldProtect) ) {
            for ( size_t i = 0; i < writepatternsize; i++ ) {
                BYTE n1 = (data[i] >> 4) & 0xf;
                BYTE n2 = data[i] & 0xf;
                if ( !writepattern[i].nibble[0].wildcard )
                    n1 = writepattern[i].nibble[0].data;

                if ( !writepattern[i].nibble[1].wildcard )
                    n2 = writepattern[i].nibble[1].data;
                data[i] = ((n1 << 4) & 0xf0) | (n2 & 0xf);
                result = VirtualProtect(data, writepatternsize, flOldProtect, &flNewProtect);
            }
        }
    }
    free(writepattern);
    return result;
}

unsigned char *patternfind(unsigned char *data, size_t datasize, size_t startindex, const char *pattern) {
    unsigned char *result = NULL;
    size_t searchpatternsize = strlen(pattern);
    LPPATTERNBYTE searchpattern = calloc(searchpatternsize, sizeof(PATTERNBYTE));

    if ( patterntransform(pattern, searchpattern, &searchpatternsize) ) {
        for ( size_t i = startindex, j = 0; i < datasize; i++ ) { //search for the pattern
            if ( (searchpattern[j].nibble[0].wildcard || searchpattern[j].nibble[0].data == ((data[i] >> 4) & 0xf))
                && (searchpattern[j].nibble[1].wildcard || searchpattern[j].nibble[1].data == (data[i] & 0xf)) ) { //check if our pattern matches the current byte

                if ( ++j == searchpatternsize ) { //everything matched
                    result = data + (i - searchpatternsize + 1);
                    break;
                }
            } else if ( j > 0 ) { //fix by Computer_Angel
                i -= j;
                j = 0; //reset current pattern position
            }
        }
    }
    free(searchpattern);
    return result;
}

unsigned char *patternsnr(unsigned char *data, size_t datasize, size_t startindex, const char *searchpattern, const char *replacepattern) {
    unsigned char *result = patternfind(data, datasize, startindex, searchpattern);
    if ( !result )
        return result;

    patternwrite(result, datasize, replacepattern);
    return result;
}
