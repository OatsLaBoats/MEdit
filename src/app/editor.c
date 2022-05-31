#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>

#include "../core/utils.h"

#include "editor.h"
#include "app_utils.h"

#define ITEM_NAME_LEN 32

typedef struct EditorItem
{
    // Functional data
    void *address;
    ValueType type;
    size_t data_size;

    void *data_buffer;
    int data_buffer_capacity;
    int data_buffer_len;

    Char8 *input_buffer;
    int input_buffer_capacity;
    int input_buffer_len;

    // UI data
    nk_bool is_selected;
    bool read_error;

    // Buffers
    Char8 name[ITEM_NAME_LEN];
    Char8 address_text[20];
} EditorItem;

int edt_init(App *app)
{
    timer_init(&app->edt.update_timer, 0.1);
    app->edt.address_value_type = I32;
    app->edt.edit_value_type = I32;

    app->edt.address_buffer_capacity = 32;
    app->edt.address_buffer = calloc(1, app->edt.address_buffer_capacity);
    return_if(app->edt.address_buffer == NULL, "Failed to allocate address input buffer", -1);

    app->edt.str_len_buffer_capacity = 32;
    app->edt.str_len_buffer = calloc(1, app->edt.str_len_buffer_capacity);
    return_if(app->edt.str_len_buffer == NULL, "Failed to allocate string length input buffer", -1);

    app->edt.name_buffer_capacity = ITEM_NAME_LEN;
    app->edt.name_buffer = calloc(1, app->edt.name_buffer_capacity);
    return_if(app->edt.name_buffer == NULL, "Failed to allocate name buffer", -1);

    app->edt.edit_buffer_capacity = kilobytes(4);
    app->edt.edit_buffer = calloc(1, app->edt.edit_buffer_capacity);
    return_if(app->edt.edit_buffer == NULL, "Failed to allocate edit input buffer", -1);

    app->edt.temp_buffer_capacity = kilobytes(30) * sizeof(Char32);
    app->edt.temp_buffer = malloc(app->edt.temp_buffer_capacity);
    return_if(app->edt.temp_buffer == NULL, "Failed to allocate temp buffer", -1);

    return_if(array_init(&app->edt.item_array, EditorItem), "Failed to initialize editor item array", -1);

    return 0;
}

static void edt_item_list(App *app, struct nk_context *ctx)
{
    struct nk_list_view view;
    uint64_t len = array_len(&app->edt.item_array);
    const char *types[] = {"i8", "i16", "i32", "i64", "u8", "u16", "u32", "u64", "f32", "f64", "utf-8", "utf-16", "utf-32"};

    float height = nk_window_get_height(ctx) - 261;

    nk_layout_row_begin(ctx, NK_DYNAMIC, 30, 6);
        nk_layout_row_push(ctx, 0.02f);
            nk_label(ctx, "", NK_TEXT_ALIGN_CENTERED | NK_TEXT_ALIGN_BOTTOM);
        nk_layout_row_push(ctx, 0.15f);
            nk_label(ctx, "Name", NK_TEXT_ALIGN_CENTERED | NK_TEXT_ALIGN_BOTTOM);
        nk_layout_row_push(ctx, 0.18f);
            nk_label(ctx, "Address", NK_TEXT_ALIGN_CENTERED | NK_TEXT_ALIGN_BOTTOM);
        nk_layout_row_push(ctx, 0.05f);
            nk_label(ctx, "Type", NK_TEXT_ALIGN_CENTERED | NK_TEXT_ALIGN_BOTTOM);
        nk_layout_row_push(ctx, 0.25f);
            nk_label(ctx, "Input", NK_TEXT_ALIGN_CENTERED | NK_TEXT_ALIGN_BOTTOM);
        nk_layout_row_push(ctx, 0.30f);
            nk_label(ctx, "Value", NK_TEXT_ALIGN_CENTERED | NK_TEXT_ALIGN_BOTTOM);
    nk_layout_row_end(ctx);

    nk_layout_row_dynamic(ctx, height, 1);
        if(nk_list_view_begin(ctx, &view, "Item list", NK_WINDOW_BORDER, 30, len))
        {
            for(int i = view.begin; i < view.end; ++i)
            {
                EditorItem *item = array_get(&app->edt.item_array, i);

                nk_layout_row_begin(ctx, NK_DYNAMIC, 30, 6);
                    nk_layout_row_push(ctx, 0.02f);
                        nk_checkbox_text(ctx, "", 0, &item->is_selected);
                    nk_layout_row_push(ctx, 0.15f);
                        nk_edit_string_zero_terminated(ctx, NK_EDIT_BOX, (char *)item->name, ITEM_NAME_LEN, nk_filter_default);
                    nk_layout_row_push(ctx, 0.18f);
                        nk_label(ctx, (char *)item->address_text, NK_TEXT_ALIGN_CENTERED | NK_TEXT_ALIGN_MIDDLE);
                    nk_layout_row_push(ctx, 0.05f);
                        nk_label(ctx, types[item->type], NK_TEXT_ALIGN_CENTERED | NK_TEXT_ALIGN_MIDDLE);
                    nk_layout_row_push(ctx, 0.25f);
                        nk_flags flags = nk_edit_string(ctx, NK_EDIT_FIELD | NK_EDIT_SIG_ENTER, (char *)item->input_buffer, &item->input_buffer_len, item->input_buffer_capacity - 1, get_plugin_filter(item->type));
                        item->input_buffer[item->input_buffer_len] = '\0';
                        if(flags & NK_EDIT_COMMITED)
                        {
                            size_t value_size = 0;
                            if(!parse_user_input(item->type, item->input_buffer, item->input_buffer_len, app->edt.temp_buffer, app->edt.temp_buffer_capacity, &value_size))
                            {
                                if(!process_write_memory(app->shared.process, item->address, app->edt.temp_buffer, item->data_size))
                                {
                                    log_message("Edited item \"%s\"", item->name);
                                }
                            }
                        }
                    nk_layout_row_push(ctx, 0.35f);
                        if(item->read_error)
                        {
                            nk_label_colored(ctx, "Read Failed!", NK_TEXT_ALIGN_CENTERED | NK_TEXT_ALIGN_MIDDLE, nk_rgb(255, 0, 0));
                        }
                        else
                        {
                            nk_edit_string(ctx, NK_EDIT_EDITOR, (char *)item->data_buffer, &item->data_buffer_len, item->data_buffer_capacity - 1, get_plugin_filter(item->type));
                        }
                nk_layout_row_end(ctx);
            }

            nk_list_view_end(&view);
        }
}

static void edt_adding_dropdowns(App *app, struct nk_context *ctx)
{
    const char *types[] = {"i8", "i16", "i32", "i64", "u8", "u16", "u32", "u64", "f32", "f64", "utf8", "utf16", "utf32"};
    const char *bases[] = {"Hex", "Dec"};

    float width = nk_window_get_width(ctx) / 2 - 22;

    nk_layout_space_begin(ctx, NK_DYNAMIC, 30, 2);
        nk_layout_space_push(ctx, nk_rect(0.24f, 0.0f, 0.25f, 0.9f));
            app->edt.address_value_type = nk_combo(ctx, types, c_array_capacity(types), app->edt.address_value_type, 30, nk_vec2(ceilf(width * 0.25f), 200));
        nk_layout_space_push(ctx, nk_rect(0.51f, 0.0f, 0.25f, 0.9f));
            app->edt.input_base_type = nk_combo(ctx, bases, c_array_capacity(bases), app->edt.input_base_type, 30, nk_vec2(ceilf(width * 0.25f), 200));
    nk_layout_space_end(ctx);
}

static void edt_address_input_field(App *app, struct nk_context *ctx)
{
    nk_layout_row_begin(ctx, NK_DYNAMIC, 30, 2);
        nk_layout_row_push(ctx, 0.20f);
            nk_label(ctx, "Address:", NK_TEXT_ALIGN_LEFT | NK_TEXT_ALIGN_MIDDLE);
        
        nk_layout_row_push(ctx, 0.80f);
            nk_plugin_filter filter = app->edt.input_base_type ? nk_filter_decimal : nk_filter_hex;
            nk_edit_string(ctx, NK_EDIT_BOX, (char *)app->edt.address_buffer, &app->edt.address_buffer_len, app->edt.address_buffer_capacity - 1, filter);
            app->edt.address_buffer[app->edt.address_buffer_len] = '\0';
    nk_layout_row_end(ctx);
}

static void edt_address_str_len_input_field(App *app, struct nk_context *ctx)
{
    ValueType vt = app->edt.address_value_type;
    if(vt != UTF8 && vt != UTF16 && vt != UTF32)
    {
        return;
    }

    nk_layout_row_begin(ctx, NK_DYNAMIC, 30, 2);
        nk_layout_row_push(ctx, 0.20f);
            nk_label(ctx, "String size:", NK_TEXT_ALIGN_LEFT | NK_TEXT_ALIGN_MIDDLE);
        
        nk_layout_row_push(ctx, 0.80f);
            nk_edit_string(ctx, NK_EDIT_BOX, (char *)app->edt.str_len_buffer, &app->edt.str_len_buffer_len, app->edt.str_len_buffer_capacity - 1, nk_filter_decimal);
            app->edt.str_len_buffer[app->edt.str_len_buffer_len] = '\0';
    nk_layout_row_end(ctx);
}

static void edt_name_input_field(App *app, struct nk_context *ctx)
{
    nk_layout_row_begin(ctx, NK_DYNAMIC, 30, 2);
        nk_layout_row_push(ctx, 0.20f);
            nk_label(ctx, "Name:", NK_TEXT_ALIGN_LEFT | NK_TEXT_ALIGN_MIDDLE);
        
        nk_layout_row_push(ctx, 0.80f);
            nk_edit_string(ctx, NK_EDIT_BOX, (char *)app->edt.name_buffer, &app->edt.name_buffer_len, app->edt.name_buffer_capacity - 1, nk_filter_default);
            app->edt.name_buffer[app->edt.name_buffer_len] = '\0';
    nk_layout_row_end(ctx);
}

static int editor_item_init(EditorItem *item, Char8 *name, void *address, ValueType data_type, size_t data_size)
{
    item->address = address;
    item->type = data_type;
    item->data_size = data_size;

    memcpy(item->name, name, ITEM_NAME_LEN);
    sprintf_s((char *)item->address_text, sizeof(item->address_text), "%p", item->address);

    item->data_buffer_capacity = item->data_size > 32 ? item->data_size : 32;
    item->data_buffer = calloc(1, item->data_buffer_capacity);
    return_if(item->data_buffer == NULL, "Failed to allocate item data buffer", -1);

    item->input_buffer_capacity = kilobytes(4);
    item->input_buffer = calloc(1, item->input_buffer_capacity);
    return_if(item->input_buffer == NULL, "Failed to allocate item input buffer", -1);

    return 0;
}

static void edt_add_button(App *app, struct nk_context *ctx)
{
    if(nk_button_label(ctx, "Add"))
    {
        if(app->edt.address_buffer[0] == '\0')
        {
            log_warning("Address field is empty");
            return;
        }

        ValueType vt = app->edt.address_value_type;
        if(vt == UTF8 || vt == UTF16 || vt == UTF32)
        {
            if(app->edt.str_len_buffer[0] == '\0')
            {
                log_warning("String size field is empty");
                return;
            }
        }

        char *tmp;
        void *address = (void *)strtoull((char *)app->edt.address_buffer, &tmp, app->edt.input_base_type ? 10 : 16);
        ValueType type = app->edt.address_value_type;
        size_t data_size = 0;

        switch(type)
        {
            case I8:
            case U8: data_size = sizeof(int8_t); break;
            
            case I16:
            case U16: data_size = sizeof(int16_t); break;

            case F32:
            case I32:
            case U32: data_size = sizeof(int32_t); break;

            case F64:
            case I64:
            case U64: data_size = sizeof(int64_t); break;

            default: data_size = strtoull((char *)app->edt.str_len_buffer, &tmp, 10); break;
        }

        EditorItem item = {0};
        do_if_s(editor_item_init(&item, app->edt.name_buffer, address, type, data_size), return);
        do_if_s(array_add(&app->edt.item_array, &item), free(item.data_buffer); free(item.input_buffer));

        app->edt.address_buffer[0] = '\0';
        app->edt.name_buffer[0] = '\0';
        app->edt.str_len_buffer[0] = '\0';

        app->edt.address_buffer_len = 0;
        app->edt.name_buffer_len = 0;
        app->edt.str_len_buffer_len = 0;

        log_message("Added new watch item %s %p", item.name, item.address);
    }
}

static void edt_remove_button(App *app, struct nk_context *ctx)
{
    if(nk_button_label(ctx, "Remove"))
    {
        Array *array = &app->edt.item_array;
        uint64_t len = array_len(array);

        for(uint64_t i = 0; i < len; ++i)
        {
            EditorItem *item = array_get(array, i);
            if(item->is_selected)
            {
                log_message("Removed watch item %s %p", item->name, item->address);
                free(item->data_buffer);
                free(item->input_buffer);
                array_remove(array, i);
                --i;
                --len;
            }
        }
    }
}

static void edt_edit_input_field(App *app, struct nk_context *ctx)
{
    nk_layout_row_begin(ctx, NK_DYNAMIC, 30, 2);
        nk_layout_row_push(ctx, 0.7);
            nk_plugin_filter filter = get_plugin_filter(app->edt.edit_value_type);
            nk_edit_string(ctx, NK_EDIT_BOX, (char *)app->edt.edit_buffer, &app->edt.edit_buffer_len, app->edt.edit_buffer_capacity - 1, filter);
            app->edt.edit_buffer[app->edt.edit_buffer_len] = '\0'; 

        float width = nk_window_get_width(ctx) / 2 - 22;
        nk_layout_row_push(ctx, 0.3);
            const char *types[] = {"i8", "i16", "i32", "i64", "u8", "u16", "u32", "u64", "f32", "f64", "utf8", "utf16", "utf32"};
            app->edt.edit_value_type = nk_combo(ctx, types, c_array_capacity(types), app->edt.edit_value_type, 30, nk_vec2(ceilf(width * 0.3f), 200));
    nk_layout_row_end(ctx);
}

static void edt_write_button(App *app, struct nk_context *ctx)
{
    if(nk_button_label(ctx, "Write"))
    {
        Array *array = &app->edt.item_array;
        uint64_t len = array_len(array);
        
        for(uint64_t i = 0; i < len; ++i)
        {
            EditorItem *item = array_get(array, i);
            if(item->is_selected && item->type == app->edt.edit_value_type)
            {
                size_t size = 0;
                if(!parse_user_input(app->edt.edit_value_type, app->edt.edit_buffer, app->edt.edit_buffer_len, app->edt.temp_buffer, app->edt.temp_buffer_capacity, &size))
                {
                    if(size > item->data_size)
                    {
                        size = item->data_size;
                    }

                    if(!process_write_memory(app->shared.process, item->address, app->edt.temp_buffer, size))
                    {
                        log_message("Edited item \"%s\"", item->name);
                    }
                }
            }
        }
    }
}

static void update_items(App *app)
{
    Array *array = &app->edt.item_array;
    uint64_t len = array_len(array);

    for(uint64_t i = 0; i < len; ++i)
    {
        EditorItem *item = array_get(array, i);

        if(process_read_memory(app->shared.process, item->address, item->data_buffer, item->data_size))
        {
            item->read_error = true;
            continue;
        }

        switch(item->type)
        {
            case I8:
            {
                int8_t v;
                memcpy(&v, item->data_buffer, sizeof(v));
                item->data_buffer_len = sprintf_s(item->data_buffer, item->data_buffer_capacity, "%d", (int)v);
            } break;

            case U8:
            {
                uint8_t v;
                memcpy(&v, item->data_buffer, sizeof(v));
                item->data_buffer_len = sprintf_s(item->data_buffer, item->data_buffer_capacity, "%d", (int)v);
            } break;

            case I16:
            {
                int16_t v;
                memcpy(&v, item->data_buffer, sizeof(v));
                item->data_buffer_len = sprintf_s(item->data_buffer, item->data_buffer_capacity, "%d", (int)v);
            } break;

            case U16:
            {
                uint16_t v;
                memcpy(&v, item->data_buffer, sizeof(v));
                item->data_buffer_len = sprintf_s(item->data_buffer, item->data_buffer_capacity, "%d", (int)v);
            } break;

            case I32:
            {
                int32_t v;
                memcpy(&v, item->data_buffer, sizeof(v));
                item->data_buffer_len = sprintf_s(item->data_buffer, item->data_buffer_capacity, "%d", v);
            } break;

            case U32:
            {
                uint32_t v;
                memcpy(&v, item->data_buffer, sizeof(v));
                item->data_buffer_len = sprintf_s(item->data_buffer, item->data_buffer_capacity, "%u", v);
            } break;

            case I64:
            {
                int64_t v;
                memcpy(&v, item->data_buffer, sizeof(v));
                item->data_buffer_len = sprintf_s(item->data_buffer, item->data_buffer_capacity, "%lld", v);
            } break;

            case U64:
            {
                uint64_t v;
                memcpy(&v, item->data_buffer, sizeof(v));
                item->data_buffer_len = sprintf_s(item->data_buffer, item->data_buffer_capacity, "%llu", v);
            } break;

            case F32:
            {
                float v;
                memcpy(&v, item->data_buffer, sizeof(v));
                item->data_buffer_len = sprintf_s(item->data_buffer, item->data_buffer_capacity, "%f", (double)v);
            } break;

            case F64:
            {
                double v;
                memcpy(&v, item->data_buffer, sizeof(v));
                item->data_buffer_len = sprintf_s(item->data_buffer, item->data_buffer_capacity, "%f", v);
            } break;

            case UTF8:
            {
                item->data_buffer_len = item->data_size;
            } break;

            case UTF16:
            {
                int len;
                log_if(convert_utf16_to_utf8(item->data_buffer, app->edt.temp_buffer, app->edt.temp_buffer_capacity, &len), "Failed to convert utf-16 to utf-8");

                memcpy(item->data_buffer, app->edt.temp_buffer, len);
                item->data_buffer_len = len;
            } break;

            case UTF32:
            {
                int len;
                log_if(convert_utf32_to_utf8(item->data_buffer, app->edt.temp_buffer, app->edt.temp_buffer_capacity, &len), "Failed to convert utf-32 to utf-8");

                memcpy(item->data_buffer, app->edt.temp_buffer, len);
                item->data_buffer_len = len;
            } break;
        }
    }
}

void edt_update(App *app, struct nk_context *ctx)
{
    if(app->shared.new_addresses_added)
    {
        app->shared.new_addresses_added = false;

        Array *items = &app->shared.scanned_addresses;
        uint64_t len = array_len(items);
        for(uint64_t i = 0; i < len; ++i)
        {
            MemoryAddress *address = array_get(items, i);
            EditorItem item = {0}; 
            editor_item_init(&item, (Char8 *)"", address->address, address->data_type, address->data_size);
            do_if_s(array_add(&app->edt.item_array, &item), free(item.data_buffer); free(item.input_buffer));
        }
    }
    
    timer_update(&app->edt.update_timer, app->shared.delta);
    if(timer_timeout(&app->edt.update_timer))
    {
        update_items(app);
    }

    nk_flags ws = NK_WINDOW_BORDER | NK_WINDOW_MOVABLE | NK_WINDOW_SCALABLE | NK_WINDOW_MINIMIZABLE | NK_WINDOW_TITLE | NK_WINDOW_NO_SCROLLBAR;
    if(nk_begin(ctx, "Editor", nk_rect(460, 1, 850, 700), ws))
    {
        nk_layout_row_dynamic(ctx, 176, 2);       
            if(nk_group_begin(ctx, "Adding", NK_WINDOW_BORDER | NK_WINDOW_NO_SCROLLBAR))
            {
                edt_adding_dropdowns(app, ctx);
                edt_address_input_field(app, ctx);
                edt_address_str_len_input_field(app, ctx);
                edt_name_input_field(app, ctx);

                nk_layout_space_begin(ctx, NK_DYNAMIC, 30, 2);
                    nk_layout_space_push(ctx, nk_rect(0.24f, 0.0f, 0.25f, 0.9f));
                        edt_add_button(app, ctx);
                    nk_layout_space_push(ctx, nk_rect(0.51f, 0.0f, 0.25f, 0.9f));
                        edt_remove_button(app, ctx);
                nk_layout_space_end(ctx);
            }
            nk_group_end(ctx);

            if(nk_group_begin(ctx, "Editing", NK_WINDOW_BORDER | NK_WINDOW_NO_SCROLLBAR))
            {
                edt_edit_input_field(app, ctx);

                nk_layout_space_begin(ctx, NK_DYNAMIC, 30, 1);
                    nk_layout_space_push(ctx, nk_rect(0.333333f, 0.0f, 0.333333f, 0.9f));
                        edt_write_button(app, ctx);
                nk_layout_space_end(ctx);
            }
            nk_group_end(ctx);

        edt_item_list(app, ctx);
    }
    nk_end(ctx);
}
