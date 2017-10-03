#include "rtl_malloc.h"

#include <phnt_windows.h>
#include <phnt.h>

void *rtl_malloc(size_t size)
{
        return RtlAllocateHeap(RtlProcessHeap(), 0, size);
}

void rtl_free(void *memblock)
{
        RtlFreeHeap(RtlProcessHeap(), 0, memblock);
}

void *rtl_calloc(size_t num, size_t size)
{
        return RtlAllocateHeap(RtlProcessHeap(), HEAP_ZERO_MEMORY, num * size);
}

void *rtl_realloc(void *memblock, size_t size)
{
        if ( !memblock )
                return rtl_malloc(size);

        if ( !size ) {
                rtl_free(memblock);
                return NULL;
        }

        return RtlReAllocateHeap(RtlProcessHeap(), 0, memblock, size);
}

void *_rtl_recalloc(void *memblock, size_t num, size_t size)
{
        if ( !memblock )
                return rtl_calloc(num, size);

        if ( !num || !size ) {
                rtl_free(memblock);
                return NULL;
        }

        return RtlReAllocateHeap(RtlProcessHeap(), HEAP_ZERO_MEMORY, memblock, num * size);
}


void *_rtl_expand(void *memblock, size_t size)
{
        return RtlReAllocateHeap(RtlProcessHeap(), HEAP_REALLOC_IN_PLACE_ONLY, memblock, size);
}

size_t _rtl_msize(void *memblock)
{
        return RtlSizeHeap(RtlProcessHeap(), 0, memblock);
}

int _rtl_heapchk(void)
{
        if ( !RtlValidateHeap(RtlProcessHeap(), 0, NULL) )
                return -1;
        return 0;
}

int _rtl_heapmin(void)
{
        if ( !RtlCompactHeap(RtlProcessHeap(), 0) )
                return -1;
        return 0;
}
