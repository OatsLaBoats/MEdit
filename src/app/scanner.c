#include <math.h>

#include "../core/utils.h"
#include "../core/thread.h"

#include "scanner.h"
#include "app_utils.h"

typedef struct AddressListItem
{
    nk_bool is_selected;
} AddressListItem;

int scn_init(App *app)
{
    return_if_s(scanner_init(&app->scn.scanner, megabytes(4)), -1);
    app->scn.value_type = I32;

    return_if(array_init(&app->scn.address_list_state, AddressListItem), "Failed to initialize address list array", -1);
    app->scn.address_list_updated = true;

    app->scn.per_address_count = 100;
    app->scn.per_address_text_capacity = 20;
    app->scn.per_address_text = calloc(app->scn.per_address_count, app->scn.per_address_text_capacity);
    return_if(app->scn.per_address_text == NULL, "Failed to allocate address list text fields", -1);

    app->scn.search_buffer_capacity = kilobytes(4);
    app->scn.search_buffer = calloc(1, app->scn.search_buffer_capacity);
    return_if(app->scn.search_buffer == NULL, "Failed to allocate search buffer", -1);

    app->scn.value_capacity = kilobytes(4) * sizeof(Char32); //extra space to store utf-32
    app->scn.value_buffer = malloc(app->scn.value_capacity);
    return_if(app->scn.value_buffer == NULL, "Failed to allocate value buffer", -1);

    return 0;
}

static void scn_address_list(App *app, struct nk_context *ctx)
{
    float height = nk_window_get_height(ctx);
    nk_layout_row_dynamic(ctx, height - 139, 1);

    struct nk_list_view view;
    if(!scanner_finished(&app->scn.scanner))
    {
        if(nk_list_view_begin(ctx, &view, "Address List", NK_WINDOW_BORDER, 25, 1))
        {
            nk_layout_row_dynamic(ctx, 25, 1);
                nk_label(ctx, "Scanning memory...", NK_TEXT_ALIGN_CENTERED);
            
            nk_list_view_end(&view);
        }

        app->scn.address_list_updated = false;
    }
    else
    {
        if(!app->scn.address_list_updated)
        {
            Array *state_array = &app->scn.address_list_state;
            ScanResult result = scanner_get_result(&app->scn.scanner);
            size_t len = array_len(result.array);

            array_extend(state_array, len);
            array_clean(state_array);
            state_array->size = len * state_array->_element;

            app->scn.address_list_updated = true;
        }

        Array *states = &app->scn.address_list_state;
        ScanResult result = scanner_get_result(&app->scn.scanner);
        int len = array_len(result.array);

        if(nk_list_view_begin(ctx, &view, "Address List", NK_WINDOW_BORDER, 25, len))
        {
            nk_layout_row_dynamic(ctx, 25, 1);
            for(int i = view.begin; i < view.end; ++i)
            {
                ScanItem *item = array_get(result.array, i);
                Char8 *text = app->scn.per_address_text + (i - view.begin) * app->scn.per_address_text_capacity;
                sprintf_s((char *)text, app->scn.per_address_text_capacity, "%p", item->address);

                AddressListItem *state = array_get(states, i);
                nk_selectable_label(ctx, (const char *)text, NK_TEXT_ALIGN_CENTERED, &state->is_selected);
            }

            nk_list_view_end(&view);
        }
    }
}

static void scn_search_user_input(App *app, struct nk_context *ctx)
{
    nk_layout_row_begin(ctx, NK_DYNAMIC, 20, 2);
        nk_layout_row_push(ctx, 0.7);
            nk_label(ctx, "Search Value", NK_TEXT_ALIGN_CENTERED | NK_TEXT_ALIGN_BOTTOM);
    nk_layout_row_end(ctx);

    nk_layout_row_begin(ctx, NK_DYNAMIC, 30, 2);
        nk_layout_row_push(ctx, 0.7);
            nk_plugin_filter filter = get_plugin_filter(app->scn.value_type);
            nk_edit_string(ctx, NK_EDIT_BOX, (char *)app->scn.search_buffer, &app->scn.search_buffer_len, app->scn.search_buffer_capacity - 1, filter);

            // Set the end of the string to 0 since nuklear doesn't
            app->scn.search_buffer[app->scn.search_buffer_len] = '\0'; 

        float width = nk_window_get_width(ctx) - 14;
        nk_layout_row_push(ctx, 0.3);
            const char *types[] = {"i8", "i16", "i32", "i64", "u8", "u16", "u32", "u64", "f32", "f64", "utf8", "utf16", "utf32"};
            app->scn.value_type = nk_combo(ctx, types, c_array_capacity(types), app->scn.value_type, 30, nk_vec2(ceilf(width * 0.3f), 200));
    nk_layout_row_end(ctx);
}

static void scn_scan_button(App *app, struct nk_context *ctx)
{
    if(nk_button_label(ctx, "Scan"))
    {
        if(app->shared.process->handle == NULL)
        {
            log_warning("Can't start a new scan if a process is not selected");
            return;
        }

        if(!scanner_finished(&app->scn.scanner))
        {
            log_warning("Can't start a new scan while an other is underway");
            return;
        }

        if(app->scn.search_buffer_len == 0)
        {
            log_warning("Can't start a new scan with a 0 bytes sized value");
            return;
        }

        if(!parse_user_input(app->scn.value_type, app->scn.search_buffer, app->scn.search_buffer_len, app->scn.value_buffer, app->scn.value_capacity, &app->scn.value_size))
        {
            scanner_scan(&app->scn.scanner, app->shared.process, app->scn.value_buffer, app->scn.value_size, app->scn.value_type, PAGE_READWRITE);
        }

        app->scn.new_scan_done = true;
    }
}

static void scn_rescan_button(App *app, struct nk_context *ctx)
{
    if(nk_button_label(ctx, "Rescan"))
    {
        if(!app->scn.new_scan_done)
        {
            log_warning("Can't start a rescan if a new scan has not been done");
            return;
        }

        if(!scanner_finished(&app->scn.scanner))
        {
            log_warning("Can't start a rescan while an other is underway");
            return;
        }

        if(app->scn.search_buffer_len == 0)
        {
            log_warning("Can't start a rescan with a 0 bytes sized value");
            return;
        }

        if(!parse_user_input(app->scn.value_type, app->scn.search_buffer, app->scn.search_buffer_len, app->scn.value_buffer, app->scn.value_capacity, &app->scn.value_size))
        {
            scanner_rescan(&app->scn.scanner, app->scn.value_buffer);
        }
    }
}

static void scn_edit_button(App *app, struct nk_context *ctx)
{
    if(nk_button_label(ctx, "Edit"))
    {
        if(!scanner_finished(&app->scn.scanner))
        {
            log_warning("Can't add value while scan is underway");
            return;
        }

        ScanResult result = scanner_get_result(&app->scn.scanner);
        uint64_t len = array_len(result.array);

        if(len == 0)
        {
            return;
        }

        array_reset(&app->shared.scanned_addresses);

        for(uint64_t i = 0; i < len; ++i)
        {
            AddressListItem *state = array_get(&app->scn.address_list_state, i);
            if(state->is_selected)
            {
                state->is_selected = false;
                ScanItem *item = array_get(result.array, i);
                MemoryAddress address = {
                    .address = item->address,
                    .data_type = result.data_type,
                    .data_size = result.data_size
                };   

                array_add(&app->shared.scanned_addresses, &address);
            }
        }

        app->shared.new_addresses_added = true;
        log_message("Added %llu memory addresses to the editor", array_len(&app->shared.scanned_addresses));
    }
}

void scn_update(App *app, struct nk_context *ctx)
{
    nk_flags ws = NK_WINDOW_BORDER | NK_WINDOW_MOVABLE | NK_WINDOW_SCALABLE | NK_WINDOW_MINIMIZABLE | NK_WINDOW_TITLE | NK_WINDOW_NO_SCROLLBAR;
    if(nk_begin(ctx, "Scanner", nk_rect(205, 1, 250, 400), ws))
    {
        scn_search_user_input(app, ctx);

        nk_layout_row_dynamic(ctx, 30, 3);
            scn_scan_button(app, ctx);
            scn_rescan_button(app, ctx);
            scn_edit_button(app, ctx);
                    
        scn_address_list(app, ctx);
    }
    nk_end(ctx);
}
