#pragma once

#pragma pack(push, 1)
typedef struct ptrlist_t_
{
        void **values;
        uint32_t *tags;
        size_t capacity;
        size_t maxCapacity;
        size_t count;
        CRITICAL_SECTION criticalSection;
} ptrlist_t;
#pragma pack(pop)

void ptrlist_lock(ptrlist_t *list);
void ptrlist_unlock(ptrlist_t *list);
void *ptrlist_at(ptrlist_t *list, size_t index, uint32_t *pTag);
bool ptrlist_create(ptrlist_t *list, size_t capacity, size_t maxCapacity);
void ptrlist_destroy(ptrlist_t *list);
size_t ptrlist_index_of(ptrlist_t *list, void *value);
bool ptrlist_add(ptrlist_t *list, void *value, uint32_t tag);
bool ptrlist_add_range(ptrlist_t *list, void **values, uint32_t *tags, size_t count);
bool ptrlist_remove_at(ptrlist_t *list, size_t index);
bool ptrlist_remove(ptrlist_t *list, void *value);
bool ptrlist_remove_range(ptrlist_t *list, size_t index, size_t count);
bool ptrlist_clear(ptrlist_t *list);
size_t ptrlist_get_count(ptrlist_t *list);
size_t ptrlist_get_capacity(ptrlist_t *list);
size_t ptrlist_get_max_capacity(ptrlist_t *list);
bool ptrlist_contains(ptrlist_t *list, void *value);
void **ptrlist_copy_values(ptrlist_t *list, size_t *count);
uint32_t *ptrlist_copy_tags(ptrlist_t *list, size_t *count);
bool ptrlist_copy(ptrlist_t *list, void ***values, uint32_t **tags, size_t *count);
void ptrlist_for(ptrlist_t *list, size_t index, size_t count, void(__cdecl *f)(void *));
void ptrlist_for_each(ptrlist_t *list, void(__cdecl *f)(void *));
void ptrlist_for_stdcall(ptrlist_t *list, size_t index, size_t count, void(__stdcall *f)(void *));
void ptrlist_for_each_stdcall(ptrlist_t *list, void(__stdcall *f)(void *));
