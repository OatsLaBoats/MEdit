#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <nuklear.h>

#include "../core/process_utils.h"
#include "../core/array.h"
#include "../core/text.h"
#include "../core/memscan.h"
#include "../core/timer.h"

typedef struct MemoryAddress
{
    void *address;
    ValueType data_type;
    size_t data_size;
} MemoryAddress;

typedef struct App
{
    // Shared
    struct
    {
        Process *process;
        
        double delta;
        
        Array scanned_addresses;
        bool new_addresses_added;
    } shared;

    // Process selector 
    struct 
    {
        ProcessArray *processes;
        Char8 **combo_state;
        DWORD *valid_pids;
        int combo_len;
        int combo_result;
    } sp;

    // Scanner
    struct 
    {
        Scanner scanner;
        ValueType value_type;

        Array address_list_state;

        int per_address_count;
        int per_address_text_capacity;
        Char8 *per_address_text;

        Char8 *search_buffer;
        int search_buffer_capacity;
        int search_buffer_len;

        void *value_buffer;
        size_t value_capacity;
        size_t value_size;

        bool new_scan_done;
        bool address_list_updated;
    } scn;

    struct
    {
        Timer update_timer;

        int input_base_type;
        ValueType address_value_type;

        Char8 *address_buffer;
        int address_buffer_capacity;
        int address_buffer_len;

        Char8 *str_len_buffer;
        int str_len_buffer_capacity;
        int str_len_buffer_len;

        Char8 *name_buffer;
        int name_buffer_capacity;
        int name_buffer_len;

        ValueType edit_value_type;
        Char8 *edit_buffer;
        int edit_buffer_capacity;
        int edit_buffer_len;

        void *temp_buffer;
        int temp_buffer_capacity;

        Array item_array;
    } edt;

    struct
    {
        int input_base_type;
        ValueType value_type;

        Char8 *input_buffer;
        int input_buffer_capacity;
        int input_buffer_len;

        Char8 *output_buffer;
        int output_buffer_capacity;
        int output_buffer_len;

        void *value_buffer;
        size_t value_capacity;
        size_t value_size;
    } ded;

    struct
    {
        int input_base_type;
        ValueType value_type;
        
        Char8 *input_buffer;
        int input_buffer_capacity;
        int input_buffer_len;

        Char8 *str_len_buffer;
        int str_len_buffer_capacity;
        int str_len_buffer_len;

        Char8 *name_buffer;
        int name_buffer_capacity;
        int name_buffer_len;

        void *temp_buffer;
        int temp_buffer_capacity;

        Array item_array;
    } wch;
} App;

int app_init(App *app);
bool app_update(App *app, struct nk_context *ctx, double delta);
void app_destroy(App *app);
