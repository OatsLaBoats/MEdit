#pragma once

#include <nuklear.h>
#include <wchar.h>
#include <stdint.h>
#include <stdbool.h>

#include "window.h"
#include "renderer.h"
#include "font.h"

typedef struct Gui
{
    Window *window;
    Renderer *renderer;
    Font *font;

    struct nk_context ctx;
    struct nk_user_font nuk_font;
    
    bool is_running;
} Gui;

LRESULT gui_proc(HWND window, UINT message, WPARAM wparam, LPARAM lparam);
struct nk_context *gui_init(Gui *gui, Window *window, Renderer *renderer, Font *font);
void gui_destroy(Gui *gui);
bool gui_process_messages(Gui *gui);
int gui_render(Gui *gui, struct nk_color color);
