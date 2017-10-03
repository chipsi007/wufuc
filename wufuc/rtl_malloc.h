#pragma once

void *rtl_malloc(size_t size);

void rtl_free(void *memblock);

void *rtl_calloc(size_t num, size_t size);

void *rtl_realloc(void *memblock, size_t size);

void *_rtl_recalloc(void *memblock, size_t num, size_t size);

void *_rtl_expand(void *memblock, size_t size);

size_t _rtl_msize(void *memblock);

int _rtl_heapchk(void);

int _rtl_heapmin(void);
