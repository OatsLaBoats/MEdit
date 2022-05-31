#pragma once

#include <stdint.h>
#include <wchar.h>
#include <Windows.h>

#include "window.h"
#include "font.h"

typedef struct Renderer
{
    HWND window;
    HDC wdc;
    HDC mdc;
    HBITMAP bitmap;
    uint32_t width;
    uint32_t height;
} Renderer;

int renderer_init(Renderer *renderer, Window *window);
void renderer_destroy(Renderer *renderer);
int renderer_resize(Renderer *renderer, uint32_t width, uint32_t height);
void renderer_start_drawing(Renderer *renderer);
int renderer_end_drawing(Renderer *renderer);

void render_fill(Renderer *renderer, COLORREF color);
int render_clipping_region(Renderer *renderer, float x, float y, float width, float height);
int render_line(Renderer *renderer, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint32_t thickness, COLORREF color);
int render_rect(Renderer *renderer, int16_t x, int16_t y, uint16_t width, uint16_t height, uint16_t rounding, uint16_t thickness, COLORREF color);
void render_filled_rect(Renderer *renderer, int16_t x, int16_t y, uint16_t width, uint16_t height, uint16_t rounding, COLORREF color);
int render_circle(Renderer *renderer, int16_t x, int16_t y, uint16_t width, uint16_t height, uint16_t thickness, COLORREF color);
void render_filled_circle(Renderer *renderer, int16_t x, int16_t y, uint16_t width, uint16_t height, COLORREF color);
int render_triangle(Renderer *renderer, int16_t x1, int16_t y1, int16_t x2, int16_t y2, int16_t x3, int16_t y3, uint16_t thickness, COLORREF color);
void render_filled_triangle(Renderer *renderer, int16_t x1, int16_t y1, int16_t x2, int16_t y2, int16_t x3, int16_t y3, COLORREF color);
int render_polygon(Renderer *renderer, short *points, int32_t row_count, uint16_t thickness, COLORREF color);
void render_filled_polygon(Renderer *renderer, short *user_points, int32_t row_count, COLORREF color);
int render_polyline(Renderer *renderer, short *points, int32_t row_count, uint16_t thickness, COLORREF color);
int render_text(Renderer *renderer, Font *font, int16_t x, int16_t y, uint16_t width, uint16_t height, const char *text, int32_t len, COLORREF bg, COLORREF fg);
int render_curve(Renderer *renderer, int16_t p1[2], int16_t p2[2], int16_t p3[2], int16_t p4[2], uint16_t thickness, COLORREF color);
void render_multi_color_rect(Renderer *renderer, int16_t x, int16_t y, uint16_t width, uint16_t height, COLORREF left, COLORREF top, COLORREF right, COLORREF bottom);
int render_image(Renderer *renderer, int16_t x, int16_t y, uint16_t width, uint16_t height, HBITMAP image);
