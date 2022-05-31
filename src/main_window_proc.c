#include "gui/gui.h"
#include "core/utils.h"

#include "window_procs.h"
#include "globals.h"
#include <stdio.h>

LRESULT CALLBACK main_window_proc(HWND window, UINT message, WPARAM wparam, LPARAM lparam)
{
    Gui *gui = get_window_data(window);
    if(gui == NULL) return DefWindowProcW(window, message, wparam, lparam);

    switch(message)
    {
        case WM_CLOSE:
        {
            g_app_is_running = false;
        } break;

        default: return gui_proc(window, message, wparam, lparam);
    }
    
    return 0;
}
