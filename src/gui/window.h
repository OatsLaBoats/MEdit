#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <Windows.h>

typedef struct Window
{
    HWND handle;
    WNDCLASSEXW class;
    uint32_t width;
    uint32_t height;
} Window;

void *get_window_data(HWND handle);
void set_window_data(HWND handle, void *data);
int get_window_count(void);

int window_init(Window *window, const wchar_t *title, uint32_t width, uint32_t height, WNDPROC callback);
void window_destroy(Window *window);
