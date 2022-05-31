#include <assert.h>
#include <stdlib.h>

#include "memscan.h"
#include "utils.h"
#include "thread.h"

typedef struct ScanThreadData
{
    Scanner *scanner;
    void *value;
    size_t value_size;
    DWORD filter;
} ScanThreadData;

int scanner_init(Scanner *scanner, size_t buffer_size)
{
    assert(scanner != NULL);
    assert(buffer_size > 0);

    clean_struct(scanner);

    scanner->regions = calloc(1, sizeof(RegionArray));
    return_if(scanner->regions == NULL, "Failed to initialize region array", -1);

    return_if(array_init(&scanner->address_array1, ScanItem), "Failed to initialize scanner array 1", -1);
    return_if(array_init(&scanner->address_array2, ScanItem), "Failed to initialize scanner array 2", -1);
    scanner->current_array = &scanner->address_array1;

    scanner->mem_buffer_size = buffer_size;
    scanner->mem_buffer = malloc(scanner->mem_buffer_size);
    return_if(scanner->mem_buffer == NULL, "Failed to allocate scanner memory buffer", -1);

    return 0;
}

bool scanner_finished(Scanner *scanner)
{
    assert(scanner != NULL);
    return !scanner->is_scanning;
}

ScanResult scanner_get_result(Scanner *scanner)
{
    assert(scanner != NULL);
    return (ScanResult){.array = scanner->current_array, .data_type = scanner->data_type, .data_size = scanner->data_size};
}

static void scan_buffer(Array *address_array, uint8_t *buffer, size_t buffer_size, size_t base_address, uint8_t *value, size_t value_size, ValueType type)
{
    size_t jump = value_size;
    if(type == UTF8)
    {
        jump = sizeof(Char8);
        int64_t a = (int64_t)buffer_size - (int64_t)value_size + jump; // So it doesn't compare beyond the edge of the buffer
        buffer_size = a < 0 ? 0 : a;
    }
    else if(type == UTF16)
    {
        jump = sizeof(Char16);
        int64_t a = (int64_t)buffer_size - (int64_t)value_size + jump;
        buffer_size = a < 0 ? 0 : a;
    }
    else if(type == UTF32)
    {
        jump = sizeof(Char32);
        int64_t a = (int64_t)buffer_size - (int64_t)value_size + jump;
        buffer_size = a < 0 ? 0 : a;
    }

    for(size_t i = 0; i < buffer_size / jump; ++i)
    {
        if(memcmp(buffer, value, value_size) == 0)
        {
            ScanItem item;
            item.address = (uint8_t *)(base_address + i * jump);
            do_if_s(array_add(address_array, &item), return);
        }

        buffer += jump;
    }
}

static int valute_type_char_size(ValueType t)
{
    switch(t)
    {
        case UTF8: return sizeof(Char8);
        case UTF16: return sizeof(Char16);
        case UTF32: return sizeof(Char32);
        default: return 0;
    }
}

static DWORD scan_thread(void *data)
{
    ScanThreadData *td = data;
    Scanner *scanner = td->scanner;
    void *value = td->value;
    size_t value_size = td->value_size;

    Array *array = &scanner->address_array1;

    clean_struct(scanner->regions);
    process_get_regions(scanner->process, scanner->regions, td->filter);
    array_reset(array);

    log_message("Scanning process with %d regions for %llu bytes", scanner->regions->len, value_size);

    for(int i = 0; i < scanner->regions->len; ++i)
    {
        Region *region = &scanner->regions->regions[i];
        if(region->size > scanner->mem_buffer_size)
        {
            size_t remaining = region->size;
            uint8_t *address = region->address;
            while(remaining >= scanner->mem_buffer_size)
            {
                if(!process_read_memory(scanner->process, address, scanner->mem_buffer, scanner->mem_buffer_size))
                {
                    scan_buffer(array, scanner->mem_buffer, scanner->mem_buffer_size, (size_t)address, value, value_size, scanner->data_type);
                }

                // Special read for strings since they may extend beyond the buffer. Might be really buggy.
                int vts = valute_type_char_size(scanner->data_type);
                if(vts != 0)
                {
                    size_t read_size = value_size * 2 - vts;
                    void *read_address = address + scanner->mem_buffer_size - value_size + vts;
                    if(!process_read_memory(scanner->process, read_address, scanner->mem_buffer, read_size))
                    {
                        scan_buffer(array, scanner->mem_buffer, read_size, (size_t)read_address, value, value_size, scanner->data_type);
                    }    
                }
                
                remaining -= scanner->mem_buffer_size;
                address += scanner->mem_buffer_size;
            }

            if(remaining != 0 && !process_read_memory(scanner->process, address, scanner->mem_buffer, remaining))
            {
                scan_buffer(array, scanner->mem_buffer, remaining, (size_t)address, value, value_size, scanner->data_type);
            }
        }
        else
        {
            if(!process_read_memory(scanner->process, region->address, scanner->mem_buffer, region->size))
            {
                scan_buffer(array, scanner->mem_buffer, region->size, (size_t)region->address, value, value_size, scanner->data_type);
            }
        }
    }

    log_message("Found %llu matches in memory", array_len(array));
    scanner->current_array = array;
    scanner->is_scanning = false;

    return 0;
}

void scanner_scan(Scanner *scanner, Process *process, void *value, size_t value_size, ValueType type, DWORD filter)
{
    assert(scanner != NULL);
    assert(process != NULL);
    assert(value != NULL);
    assert(value_size != 0);

    scanner->is_scanning = true;
    scanner->process = process;
    scanner->data_type = type;
    scanner->data_size = value_size;

    static ScanThreadData td;
    td = (ScanThreadData){
        .scanner = scanner,
        .value = value,
        .value_size = value_size,
        .filter = filter
    };

    create_thread(scan_thread, &td);
}

static void scan_addresses(Process *process, Array *src, Array *dst, void *temp_buffer, uint8_t *value, size_t value_size)
{
    int len = array_len(src);
    for(int i = 0; i < len; ++i)
    {
        ScanItem *item = array_get(src, i);
        if(!process_read_memory(process, item->address, temp_buffer, value_size))
        {
            if(memcmp(temp_buffer, value, value_size) == 0)
            {
                array_add(dst, item);
            }
        }
    }
}

static Array *swap_array(Scanner *scanner)
{
    return scanner->current_array == &scanner->address_array1 ? &scanner->address_array2 : &scanner->address_array1;
}

static DWORD rescan_thread(void *data)
{
    ScanThreadData *td = data;
    Scanner *scanner = td->scanner;
    void *value = td->value;
    size_t value_size = td->value_size;

    log_message("Rescanning memory for %llu byte value", value_size);

    Array *src = scanner->current_array;
    Array *dst = swap_array(scanner);
    array_reset(dst);

    scan_addresses(scanner->process, src, dst, scanner->mem_buffer, value, value_size);

    log_message("Found %llu matches in memory", array_len(dst));
    scanner->current_array = dst;
    scanner->is_scanning = false;

    return 0;
}

void scanner_rescan(Scanner *scanner, void *value)
{
    assert(scanner != NULL);
    assert(value != NULL);

    scanner->is_scanning = true;

    static ScanThreadData td;
    td = (ScanThreadData){
        .scanner = scanner,
        .value = value,
        .value_size = scanner->data_size
    };

    create_thread(rescan_thread, &td);
}

