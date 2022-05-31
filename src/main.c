#include <stddef.h>
#include <stdio.h>
#include <locale.h>
#include <wchar.h>

#include "core/utils.h"
#include "gui/gui.h"
#include "app/app.h"

#include "window_procs.h"

// Globals
bool g_medit_error = false;
bool g_app_is_running = true;

int main(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    int fps = 120;

    setlocale(LC_ALL, "en_US.utf8");

    Window main_window = {0};
    return_if_s(window_init(&main_window, L"MEdit", 1500, 900, main_window_proc), -1)

    Renderer main_renderer = {0};
    return_if_s(renderer_init(&main_renderer, &main_window), -1);

    Font font = {0};
    return_if_s(font_init(&font, "Consolas", 14), -1);

    Gui gui = {0};
    struct nk_context *ctx = gui_init(&gui, &main_window, &main_renderer, &font);
    return_if_s(ctx == NULL, -1);

    set_window_data(main_window.handle, &gui);
    
    App app = {0};
    do_if_s(app_init(&app), gui_destroy(&gui); return -1);

    int result = 0;

    double delta = 0.0;
    double frame_start = get_elapsed_milliseconds();
    while(!g_medit_error && g_app_is_running && get_window_count() > 0)
    {
        // Check for alt-f4
        if(GetKeyState(VK_MENU) & (1 << 15) && GetKeyState(VK_F4) & (1 << 15)) break;

        if(gui_process_messages(&gui))
        {
            do_if_s(!app_update(&app, ctx, delta), break);
            do_if_s(gui_render(&gui, nk_rgb(30,30,30)), result = -1; break);
        }

        double frame_end = get_elapsed_milliseconds();
        double sleep_ms = (1000.0 / (double)fps) - (frame_end - frame_start);

        if(sleep_ms > 0.0)
        {
            Sleep((DWORD)sleep_ms);
        }

        double loop_end = get_elapsed_milliseconds();
        delta = (loop_end - frame_start) / 1000.0;
        frame_start = loop_end;
    }

    app_destroy(&app);
    
    return result;
}

