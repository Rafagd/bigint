#include "array.h"

#include<stdio.h>
#include<string.h>

#define MAX(a, b) (((a) > (b)) ? (a) : (b))

array_t* array_new(size_t item_size)
{
    array_t* new_array = malloc(sizeof(array_t));

    if (new_array == NULL) {
        return NULL;
    }

    new_array->items     = NULL;
    new_array->item_size = item_size;
    new_array->size      = 0;
    new_array->capacity  = 0;

    return new_array;
}

void array_delete(array_t* array)
{
    if (array->items != 0) {
        free(array->items);
    }
    free(array);
}

bool array_resize(array_t* array, size_t new_size, array_align_t align)
{
    // Nothing to do.
    if (new_size == array->size) {
        return true;
    }

    size_t u8_new_size = new_size    * array->item_size;
    size_t u8_old_size = array->size * array->item_size;

    if (new_size < array->size) {
        if (align == ARRAY_ALIGN_RIGHT) {
            memmove(ARRAY_GET(array, 0), ARRAY_GET_R(array, -1), u8_new_size);
        }
        array->size = new_size;
        return true;
    }

    size_t size_diff    = new_size - array->size;
    size_t u8_size_diff = size_diff * array->item_size;

    if (new_size < array->capacity) {

        if (align == ARRAY_ALIGN_LEFT) {
            memset(ARRAY_GET_R(array, -1), 0, u8_size_diff);

        } else {
            memmove(
                ARRAY_GET(array, size_diff),  // DST is the ptr STARTING at diff.
                ARRAY_GET(array, 0),
                u8_new_size
            );
            memset(ARRAY_GET(array, 0), 0, u8_size_diff);
        }

        array->size = new_size;
        return true;
    }

    uint8_t* new_items = malloc(u8_new_size);
    if (new_items == NULL) {
        return false;
    }

    memset(new_items, 0, u8_size_diff);

    if (align == ARRAY_ALIGN_LEFT) {
        memmove(new_items, ARRAY_GET(array, 0), u8_old_size);

    } else {
        memmove(new_items + u8_size_diff, ARRAY_GET(array, 0), u8_new_size);
    }

    array->size = new_size;

    if (array->items != NULL) {
        free(array->items);
    }

    array->items    = new_items;
    array->capacity = new_size;
    array->size     = new_size;
    return true;
}

bool array_set(array_t* array, size_t index, void* value)
{
    if (index >= array->size) {
        return false;
    }

    uint8_t* position = array->items + index * array->item_size;
    memmove(position, value, array->item_size);
    return true;
}

void* array_get(array_t* array, size_t index)
{
    if (index >= array->size) {
        return NULL;
    }

    return array->items + index * array->item_size;
}

bool array_equals(array_t* a, array_t* b)
{
    if (a->size != b->size || a->item_size != b->item_size) {
        return false;
    }

    if (a->size == 0 && b->size == 0) {
        return true;
    }

    return memcmp(a->items, b->items, a->size * a->item_size) == 0;
}

bool array_push_back(array_t* array, void* item)
{
    // Update the size
    if (!ARRAY_RESIZE(array, array->size + 1)) {
        return false;
    }

    // Update the last position
    return array_set(array, array->size - 1, item);
}

void* array_pop_back(array_t* array)
{
    size_t new_size = array->size - 1;

    // Copy the last item
    uint8_t* item = malloc(array->item_size);
    memcpy(item, array_get(array, new_size), array->item_size);

    // Resize and return item
    if (!ARRAY_RESIZE(array, new_size)) {
        return NULL;
    }

    return item;
}

bool array_push_front(array_t* array, void* item)
{
    size_t old_size = array->size;

    // Update the size
    if (!ARRAY_RESIZE(array, array->size + 1)) {
        return false;
    }

    // Shift the array to the right
    memmove(
        array_get(array, 1),        // dst:  arrays[1] 
        array_get(array, 0),        // src:  arrays[0]
        old_size * array->item_size // size: old_size
    );

    // Add item to the start
    return array_set(array, 0, item);
}

void* array_pop_front(array_t* array)
{
    // Copy the first item
    uint8_t* item = malloc(array->item_size);
    memcpy(item, array_get(array, 0), array->item_size);
    
    size_t new_size = array->size - 1;

    // Shift the array to the left
    memmove(
        array_get(array, 0),        // dst: arrays[0]
        array_get(array, 1),        // src: arrays[1]
        new_size * array->item_size // size: old_size - 1
    );

    // Resize and return item
    if (!ARRAY_RESIZE(array, new_size)) {
        return NULL;
    }

    return item;
}

void array_printf(array_t* array, const char* pattern)
{
    printf("[");
    for (size_t i = 0; i < array->size; i++) {
        if (i > 0) {
            printf(",");
        }
        printf(" ");
        printf(pattern, array_get(array, i));
    }
    printf("]\n");
}

void memdump(uint8_t* ptr, size_t size)
{
    for (size_t i = 0; i < size; i++) {
        printf("%02X ", ptr[i]);
    }
    printf("\n");
}
