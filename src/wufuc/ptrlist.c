#include "stdafx.h"
#include "ptrlist.h"

void ptrlist_lock(ptrlist_t *list)
{
        EnterCriticalSection(&list->criticalSection);
}

void ptrlist_unlock(ptrlist_t *list)
{
        LeaveCriticalSection(&list->criticalSection);
}

void *ptrlist_at(ptrlist_t *list, size_t index, uint32_t *pTag)
{
        void *result;

        ptrlist_lock(list);
        result = list->values[index];
        if ( pTag )
                *pTag = list->tags[index];
        ptrlist_unlock(list);
        return result;
}

bool ptrlist_create(ptrlist_t *list, size_t capacity, size_t maxCapacity)
{
        bool result = false;
        size_t c;
        size_t vsize;
        size_t tsize;
        void *tmp;

        if ( !list || capacity > maxCapacity )
                return result;

        c = capacity ? capacity :
                (maxCapacity ? min(maxCapacity, 16) : 16);
        vsize = c * (sizeof *list->values);
        tsize = c * (sizeof *list->tags);

        InitializeCriticalSection(&list->criticalSection);
        ptrlist_lock(list);

        tmp = malloc(vsize + tsize);
        if ( tmp ) {
                ZeroMemory(tmp, vsize + tsize);
                list->values = tmp;
                list->tags = OffsetToPointer(tmp, vsize);
                list->capacity = c;
                list->maxCapacity = maxCapacity;
                list->count = 0;
                result = true;
        }
        ptrlist_unlock(list);
        if ( !result )
                DeleteCriticalSection(&list->criticalSection);
        return result;
}

void ptrlist_destroy(ptrlist_t *list)
{
        if ( !list ) return;

        ptrlist_lock(list);

        free(list->values);
        list->values = NULL;
        list->tags = NULL;

        list->count = 0;
        list->capacity = 0;
        list->maxCapacity = 0;

        ptrlist_unlock(list);
        DeleteCriticalSection(&list->criticalSection);
}

size_t ptrlist_index_of(ptrlist_t *list, void *value)
{
        size_t result = -1;

        if ( !list || !value )
                return result;

        ptrlist_lock(list);
        for ( size_t i = 0; i < list->count; i++ ) {
                if ( list->values[i] == value ) {
                        result = i;
                        break;
                }
        }
        ptrlist_unlock(list);
        return result;
}

bool ptrlist_add(ptrlist_t *list, void *value, uint32_t tag)
{
        bool result = false;
        size_t newCapacity;
        size_t diff;
        size_t vsize;
        size_t tsize;
        void **tmp1;
        uint32_t *tmp2;

        if ( !list || !value )
                return result;

        ptrlist_lock(list);

        if ( list->count >= list->capacity ) {
                newCapacity = list->count;
                if ( list->maxCapacity ) {
                        diff = list->maxCapacity - list->capacity;
                        if ( !diff )
                                goto leave;
                        newCapacity += min(diff, 16);
                } else {
                        newCapacity += 16;
                }
                vsize = newCapacity * (sizeof *list->values);
                tsize = newCapacity * (sizeof *list->tags);

                tmp1 = malloc(vsize + tsize);

                if ( !tmp1 )
                        goto leave;

                ZeroMemory(tmp1, vsize);

                tmp2 = OffsetToPointer(tmp1, vsize);
                ZeroMemory(tmp2, tsize);

                if ( memmove_s(tmp1, vsize, list->values, list->count * (sizeof *list->values))
                        || memmove_s(tmp2, tsize, list->tags, list->count * (sizeof *list->tags)) ) {

                        free(tmp1);
                        goto leave;
                }
                list->values = tmp1;
                list->tags = tmp2;
                list->capacity = newCapacity;
        }
        list->values[list->count] = value;
        list->tags[list->count] = tag;
        list->count++;
        result = true;
leave:
        ptrlist_unlock(list);
        return result;
}

bool ptrlist_add_range(ptrlist_t *list, void **values, uint32_t *tags, size_t count)
{
        bool result = true;

        if ( !list || !values || !count )
                return false;

        ptrlist_lock(list);
        if ( list->count + count <= list->maxCapacity ) {
                for ( size_t i = 0; result && i < count; i++ )
                        result = ptrlist_add(list, values[i], tags ? tags[i] : 0);
        } else {
                result = false;
        }
        ptrlist_unlock(list);
        return result;
}

bool ptrlist_remove_at(ptrlist_t *list, size_t index)
{
        bool result = false;

        if ( !list ) return result;

        ptrlist_lock(list);
        if ( index <= list->count - 1 ) {
                for ( size_t i = index; i < list->count - 1; i++ )
                        list->values[i] = list->values[i + 1];

                list->values[list->count--] = NULL;
                result = true;
        }
        ptrlist_unlock(list);
        return result;
}

bool ptrlist_remove(ptrlist_t *list, void *value)
{
        size_t index;
        bool result = false;

        if ( !list || !value )
                return result;

        ptrlist_lock(list);
        index = ptrlist_index_of(list, value);
        if ( index != -1 )
                result = ptrlist_remove_at(list, index);
        ptrlist_unlock(list);
        return result;
}

bool ptrlist_remove_range(ptrlist_t *list, size_t index, size_t count)
{
        bool result = true;

        if ( !list || !count )
                return false;

        ptrlist_lock(list);
        if ( index <= list->count - 1
                && index + count <= list->count ) {

                for ( size_t i = 0; result && i < count; i++ )
                        result = ptrlist_remove_at(list, index);
        } else {
                result = false;
        }
        ptrlist_unlock(list);
        return result;
}

bool ptrlist_clear(ptrlist_t *list)
{
        bool result = false;

        if ( !list ) return result;

        ptrlist_lock(list);
        result = ptrlist_remove_range(list, 0, list->count);
        ptrlist_unlock(list);
        return result;
}

size_t ptrlist_get_count(ptrlist_t *list)
{
        size_t result = -1;

        if ( !list ) return result;

        ptrlist_lock(list);
        result = list->count;
        ptrlist_unlock(list);
        return result;
}

size_t ptrlist_get_capacity(ptrlist_t *list)
{
        size_t result = -1;

        if ( !list ) return result;

        ptrlist_lock(list);
        result = list->capacity;
        ptrlist_unlock(list);
        return result;
}

size_t ptrlist_get_max_capacity(ptrlist_t *list)
{
        size_t result = -1;

        if ( !list ) return result;

        ptrlist_lock(list);
        result = list->maxCapacity;
        ptrlist_unlock(list);
        return result;
}

bool ptrlist_contains(ptrlist_t *list, void *value)
{
        return ptrlist_index_of(list, value) != -1;
}

void **ptrlist_copy_values(ptrlist_t *list, size_t *count)
{
        void **result = NULL;
        size_t size;
        size_t c;

        if ( !list || !count )
                return result;

        ptrlist_lock(list);
        c = list->count;
        if ( !c ) goto leave;

        size = c * (sizeof *list->values);
        result = malloc(c * (sizeof *list->values));
        if ( result ) {
                if ( !memcpy_s(result, size, list->values, size) ) {
                        *count = c;
                } else {
                        free(result);
                        result = NULL;
                }
        }
leave:
        ptrlist_unlock(list);
        return result;
}

uint32_t *ptrlist_copy_tags(ptrlist_t *list, size_t *count)
{
        uint32_t *result = NULL;
        size_t size;
        size_t c;

        if ( !list || !count )
                return result;

        ptrlist_lock(list);
        c = list->count;
        if ( !c ) goto leave;

        size = c * (sizeof *list->tags);
        result = malloc(c * (sizeof *list->tags));
        if ( result ) {
                if ( !memcpy_s(result, size, list->tags, size) ) {
                        *count = c;
                } else {
                        free(result);
                        result = NULL;
                }
        }
leave:
        ptrlist_unlock(list);
        return result;
}

bool ptrlist_copy(ptrlist_t *list, void ***values, uint32_t **tags, size_t *count)
{
        bool result = false;
        void **v;
        uint32_t *t;
        size_t c;

        if ( !values || !tags || !count )
                return result;

        ptrlist_lock(list);
        v = ptrlist_copy_values(list, &c);
        if ( !v ) goto leave;

        t = ptrlist_copy_tags(list, &c);
        if ( !t ) {
                free(v);
                goto leave;
        }
        *values = v;
        *tags = t;
        *count = c;
        result = true;
leave:
        ptrlist_unlock(list);
        return result;
}

void ptrlist_for(ptrlist_t *list, size_t index, size_t count, void(__cdecl *f)(void *))
{
        if ( !list || !f ) return;

        ptrlist_lock(list);
        if ( index + count <= list->count ) {
                for ( size_t i = index; i < count; i++ )
                        f(list->values[i]);
        }
        ptrlist_unlock(list);
}

void ptrlist_for_each(ptrlist_t *list, void(__cdecl *f)(void *))
{
        if ( !list || !f ) return;

        ptrlist_lock(list);
        ptrlist_for(list, 0, list->count, f);
        ptrlist_unlock(list);
}

void ptrlist_for_stdcall(ptrlist_t *list, size_t index, size_t count, void(__stdcall *f)(void *))
{
        if ( !list || !f ) return;

        ptrlist_lock(list);
        if ( index + count <= list->count ) {
                for ( size_t i = index; i < count; i++ )
                        f(list->values[i]);
        }
        ptrlist_unlock(list);
}

void ptrlist_for_each_stdcall(ptrlist_t *list, void(__stdcall *f)(void *))
{
        if ( !list || !f ) return;

        ptrlist_lock(list);
        ptrlist_for_stdcall(list, 0, list->count, f);
        ptrlist_unlock(list);
}
