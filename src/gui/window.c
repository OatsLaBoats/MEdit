#include <assert.h>

#include "../core/utils.h"

#include "window.h"

typedef struct WindowData
{
    HANDLE handle;
    void *data;
} WindowData;

static int g_window_count;
static WindowData g_windows[64];

static int add_window(HWND handle, void *data)
{
    return_if(g_window_count == 64, "Maximum number of windows created", -1);

    int index = (uint64_t)handle % 64;
    for(;;)
    {
        if(g_windows[index].handle == NULL)
        {
            g_windows[index] = (WindowData){
                .handle = handle,
                .data = data
            };

            ++g_window_count;
            break;
        }
        
        index = (index + 1) % 64;
    }

    return 0;
}

static void remove_window(HWND handle)
{
    int index = (uint64_t)handle % 64;
    int start_index = index;
    for(;;)
    {
        if(g_windows[index].handle == handle)
        {
            g_windows[index].handle = NULL;
            g_windows[index].data = NULL;
            --g_window_count;
            break;
        }

        index = (index + 1) % 64;

        if(index == start_index)
        {
            break;
        }
    }
}

void *get_window_data(HWND handle)
{
    int index = (uint64_t)handle % 64;
    int start_index = index;
    for(;;)
    {
        if(g_windows[index].handle == handle)
        {
            return g_windows[index].data;
        }

        index = (index + 1) % 64;

        if(index == start_index)
        {
            break;
        }
    }

    return NULL;
}

void set_window_data(HWND handle, void *data)
{
    int index = (uint64_t)handle % 64;
    int start_index = index;
    for(;;)
    {
        if(g_windows[index].handle == handle)
        {
            g_windows[index].data = data;
            break;
        }

        index = (index + 1) % 64;

        if(index == start_index)
        {
            break;
        }
    }
}

int get_window_count(void)
{
    return g_window_count;
}

int window_init(Window *window, const wchar_t *title, uint32_t width, uint32_t height, WNDPROC callback)
{
    assert(window != NULL);
    assert(title != NULL);
    assert(width != 0);
    assert(height != 0);
    assert(callback != NULL);

    HWND window_handle = NULL;

    HINSTANCE instance_handle = GetModuleHandleW(NULL);
    return_if_win(instance_handle == NULL, "Failed to get executable module handle", -1);

    HICON app_icon = LoadIconA(NULL, IDI_APPLICATION);
    return_if_win(app_icon == NULL, "Failed to load app icon", -1);

    HCURSOR app_cursor = LoadCursorA(NULL, IDC_ARROW);
    return_if_win(app_cursor == NULL, "Failed to load app cursor", -1);

    WNDCLASSEXW wc = {
        .cbSize = sizeof(WNDCLASSEXW),
        .style = CS_DBLCLKS,
        .lpfnWndProc = callback,
        .cbClsExtra = 0,
        .cbWndExtra = 0,
        .hInstance = instance_handle,
        .hIcon = app_icon,
        .hCursor = app_cursor,
        .hbrBackground = NULL,
        .lpszMenuName = NULL,
        .lpszClassName = L"MeditWindowClass",
        .hIconSm = NULL
    };

    return_if_win(RegisterClassExW(&wc) == 0, "Failed to register window class", -1);

    DWORD style = WS_OVERLAPPEDWINDOW;
    DWORD ex_style = WS_EX_APPWINDOW;
    RECT window_rect = {0, 0, width, height};

    jump_if_win(!AdjustWindowRectEx(&window_rect, style, FALSE, ex_style), 
                "Failed to resize window rect for desired client area", cleanup);

    window_handle = CreateWindowExW(ex_style, 
                                    wc.lpszClassName, 
                                    title, 
                                    style | WS_VISIBLE, 
                                    CW_USEDEFAULT, CW_USEDEFAULT, 
                                    window_rect.right - window_rect.left,
                                    window_rect.bottom - window_rect.top,
                                    NULL, NULL,
                                    wc.hInstance,
                                    NULL);
    jump_if_win(window_handle == NULL, "Failed to open window", cleanup);
    jump_if(add_window(window_handle, NULL), "Failed to add window to window table", cleanup);

    window->width = width;
    window->height = height;
    window->class = wc;
    window->handle = window_handle;

    return 0;

    cleanup:
    if_valid(window_handle != NULL, log_if_win(!DestroyWindow(window_handle), "Failed to destory window"));
    log_if_win(!UnregisterClassW(wc.lpszClassName, instance_handle), "Failed to unregister window class");

    return -1;
}

void window_destroy(Window *window)
{
    if_valid(window->handle != NULL, log_if_win(!DestroyWindow(window->handle), "Failed to destroy window"));
    log_if_win(!UnregisterClassW(window->class.lpszClassName, window->class.hInstance), "Failed to unregister window class");
    remove_window(window->handle);
    clean_struct(window);
}
