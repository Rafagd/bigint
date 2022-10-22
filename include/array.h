#ifndef ARRAY_H
#define ARRAY_H

#include<stdbool.h>
#include<stdint.h>
#include<stdlib.h>

typedef enum array_align_e
{
    ARRAY_ALIGN_LEFT  = 0,
    ARRAY_ALIGN_RIGHT,
} array_align_t;

typedef struct array_s
{
    uint8_t* items;
    size_t   item_size;
    size_t   size;
    size_t   capacity;
} array_t;

#define ARRAY_NEW(type) array_new(sizeof(type))
array_t* array_new(size_t item_size);
void     array_delete(array_t* array);

#define ARRAY_RESIZE(array, new_size)   array_resize(array, new_size, ARRAY_ALIGN_LEFT)
#define ARRAY_RESIZE_L(array, new_size) array_resize(array, new_size, ARRAY_ALIGN_LEFT)
#define ARRAY_RESIZE_R(array, new_size) array_resize(array, new_size, ARRAY_ALIGN_RIGHT)
bool array_resize(array_t* array, size_t new_size, array_align_t align);

bool  array_set(array_t* array, size_t index, void* value);

// Macro version does no bounds checking
#define ARRAY_GET(array, index)   ((array)->items + (index) * (array)->item_size)
#define ARRAY_GET_L(array, index) ((array)->items + (index) * (array)->item_size)
#define ARRAY_GET_R(array, index) ((array)->items + ((array)->size - (index + 1)) * (array)->item_size)

void* array_get(array_t* array, size_t index);

bool  array_equals(array_t* a, array_t* b);

bool  array_push_back(array_t* array, void* item);
void* array_pop_back(array_t* array);

bool  array_push_front(array_t* array, void* item);
void* array_pop_front(array_t* array);

void array_printf(array_t* array, const char* pattern);

void memdump(uint8_t *ptr, size_t size);

#endif // ARRAY_H
