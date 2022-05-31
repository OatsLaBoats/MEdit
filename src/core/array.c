#include "array.h"
#include "utils.h"

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

int array_custom_init(Array *array, size_t element, size_t capacity)
{
    assert(array != NULL);
    assert(element != 0);
    assert(capacity != 0);

    array->_bytes = malloc(element * capacity);
    if(array->_bytes == NULL)
    {
        log_error("Failed to allocate array with element size %llu and capacity size %llu", element, capacity);
        return -1;
    }

    array->_element = element;
    array->capacity = element * capacity;
    array->size = 0;

    return 0;
}

int array_add(Array *array, void *item)
{
    assert(array != NULL);
    assert(item != NULL);

    if(array->size >= array->capacity)
    {
        size_t new_capacity = array->capacity * 2;
        void *tmp = realloc(array->_bytes, new_capacity);
        return_if(tmp == NULL, "Failed to reallocate array buffer", -1);
        
        array->_bytes = tmp;
        array->capacity = new_capacity;
    }

    memcpy(array->_bytes + array->size, item, array->_element);
    array->size += array->_element;

    return 0;
}

void array_remove(Array *array, uint64_t index)
{
    assert(array != NULL);
    assert(array->_bytes != NULL);
    assert(array->size != 0);

    uint64_t len = array_len(array);
    assert(index < len);

    if(index == len - 1)
    {
        array->size -= array->_element;
        return;
    }
    
    void *dst = array_get(array, index);
    void *src = array_get(array, index + 1);
    uint64_t size = array->size - (index + 1) * array->_element;
    array->size -= array->_element;

    memmove(dst, src, size);
}

uint64_t array_len(Array *array)
{
    assert(array != NULL);
    return array->size / array->_element;
}
 
void *array_get(Array *array, uint64_t index)
{
    assert(array != NULL);
    assert(array->_bytes != NULL);
    assert(index >= 0);
    assert(index < array_len(array));

    return array->_bytes + index * array->_element;
}

void array_reset(Array *array)
{
    assert(array != NULL);
    array->size = 0;
}

void array_clean(Array *array)
{
    assert(array != NULL);
    array->size = 0;
    memset(array->_bytes, 0, array->capacity);
}

int array_extend(Array *array, uint64_t elements)
{
    assert(array);
    return_if_s(array->capacity / array->_element > elements, 0);

    size_t new_capacity = elements * array->_element;
    void *tmp = realloc(array->_bytes, new_capacity);
    return_if(tmp == NULL, "Failed to reallocate array", -1);

    array->_bytes = tmp;
    array->capacity = new_capacity;
    
    return 0;
}
