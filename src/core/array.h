#pragma once

#include <stdint.h>

typedef struct Array
{
    uint8_t *_bytes;
    size_t _element;
    size_t capacity;
    size_t size;
} Array;

#define array_init(array_ptr, T) array_custom_init(array_ptr, sizeof(T), 32) 
int array_custom_init(Array *array, size_t element, size_t capacity);
int array_add(Array *array, void *item);
void array_remove(Array *array, uint64_t index);
uint64_t array_len(Array *array);
void *array_get(Array *array, uint64_t index);
void array_reset(Array *array);
void array_clean(Array *array);
int array_extend(Array *array, uint64_t elements);
