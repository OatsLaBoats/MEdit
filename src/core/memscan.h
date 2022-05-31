#pragma once

#include <stdatomic.h>
#include <stdbool.h>
#include <stddef.h>

#include "array.h"
#include "process_utils.h"
#include "text.h"

typedef enum ValueType 
{
    I8, I16, I32, I64,
    U8, U16, U32, U64,
    F32, F64,
    UTF8, UTF16, UTF32
} ValueType;

typedef struct ScanItem
{
    void *address;
} ScanItem;

typedef struct Scanner
{
    atomic_bool is_scanning;
    
    Process *process;
    RegionArray *regions;

    ValueType data_type;
    size_t data_size;

    Array *current_array;
    Array address_array1;
    Array address_array2;

    void *mem_buffer;
    size_t mem_buffer_size;
} Scanner;

typedef struct ScanResult
{
    Array *array;
    ValueType data_type;
    size_t data_size;
} ScanResult;

int scanner_init(Scanner *scanner, size_t buffer_size);
bool scanner_finished(Scanner *scanner);
ScanResult scanner_get_result(Scanner *scanner);
void scanner_scan(Scanner *scanner, Process *process, void *value, size_t value_size, ValueType type, DWORD filter);
void scanner_rescan(Scanner *scanner, void *value);

