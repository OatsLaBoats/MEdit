#include <assert.h>

#include "../core/utils.h"

#include "renderer.h"

int renderer_init(Renderer *renderer, Window *window)
{
    assert(renderer != NULL);
    assert(window != NULL);

    HDC window_dc = NULL;
    HBITMAP bitmap = NULL;
    HDC memory_dc = NULL;

    window_dc = GetDC(window->handle);
    bitmap = CreateCompatibleBitmap(window_dc, window->width, window->height);
    jump_if(bitmap == NULL, "Failed to create renderer bitmap", cleanup);

    memory_dc = CreateCompatibleDC(window_dc);
    jump_if(memory_dc == NULL, "Failed to create memory dc", cleanup);

    SelectObject(memory_dc, bitmap);

    renderer->width = window->width;
    renderer->height = window->height;
    renderer->wdc = window_dc;
    renderer->bitmap = bitmap;
    renderer->mdc = memory_dc;
    renderer->window = window->handle;

    return 0;

    cleanup:
    if(bitmap != NULL) DeleteObject(bitmap);
    if(memory_dc != NULL) DeleteDC(memory_dc);
    if(window_dc != NULL) ReleaseDC(window->handle, window_dc);

    return -1;
}

void renderer_destroy(Renderer *renderer)
{
    if(renderer->bitmap != NULL) DeleteObject(renderer->bitmap);
    if(renderer->mdc != NULL) DeleteDC(renderer->mdc);
    if(renderer->wdc != NULL) ReleaseDC(renderer->window, renderer->wdc);
    clean_struct(renderer);
}

int renderer_resize(Renderer *renderer, uint32_t width, uint32_t height)
{
    assert(renderer != NULL);
    assert(renderer->bitmap != NULL);
    assert(renderer->mdc != NULL);
    assert(width != 0);
    assert(height != 0);

    if(width == renderer->width && height == renderer->height)
    {
        return 0;
    }

    DeleteObject(renderer->bitmap);

    renderer->bitmap = CreateCompatibleBitmap(renderer->wdc, width, height);
    return_if(renderer->bitmap == NULL, "Failed to create renderer bitmap", -1);

    renderer->width = width;
    renderer->height = height;

    SelectObject(renderer->mdc, renderer->bitmap);

    return 0;
}

void renderer_start_drawing(Renderer *renderer)
{
    assert(renderer != NULL);
    assert(renderer->mdc != NULL);

    SelectObject(renderer->mdc, GetStockObject(DC_PEN));
    SelectObject(renderer->mdc, GetStockObject(DC_BRUSH));
}

int renderer_end_drawing(Renderer *renderer)
{
    assert(renderer != NULL);
    assert(renderer->wdc != NULL);
    assert(renderer->mdc != NULL);

    return_if_win(!BitBlt(renderer->wdc, 0, 0, renderer->width, renderer->height, renderer->mdc, 0, 0, SRCCOPY), "Failed to draw on the renderer", -1);
    return 0;
}

void render_fill(Renderer *renderer, COLORREF color)
{
    assert(renderer != NULL);
    assert(renderer->mdc != NULL);

    RECT rect;
    SetRect(&rect, 0, 0, renderer->width, renderer->height);
    SetBkColor(renderer->mdc, color);
    ExtTextOutW(renderer->mdc, 0, 0, ETO_OPAQUE, &rect, NULL, 0, NULL);
}

int render_clipping_region(Renderer *renderer, float x, float y, float width, float height)
{
    assert(renderer != NULL);
    assert(renderer->mdc != NULL);

    SelectClipRgn(renderer->mdc, NULL);
    return_if(IntersectClipRect(renderer->mdc, (int)x, (int)y, (int)(x + width + 1), (int)(y + height + 1)) == ERROR, "Failed to select new region", -1);

    return 0;
}

int render_line(Renderer *renderer, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint32_t thickness, COLORREF color)
{
    assert(renderer != NULL);
    assert(renderer->mdc != NULL);

    HPEN pen = NULL;
    if(thickness == 1)
    {
        SetDCPenColor(renderer->mdc, color);
    }
    else
    {
        pen = CreatePen(PS_SOLID, thickness, color);
        return_if(pen == NULL, "Failed to create pen", -1);
        SelectObject(renderer->mdc, pen);
    }

    MoveToEx(renderer->mdc, x1, y1, NULL);
    LineTo(renderer->mdc, x2, y2);

    if(pen != NULL)
    {
        SelectObject(renderer->mdc, GetStockObject(DC_PEN));
        DeleteObject(pen);
    }

    return 0;
}

int render_rect(Renderer *renderer, int16_t x, int16_t y, uint16_t width, uint16_t height, uint16_t rounding, uint16_t thickness, COLORREF color)
{
    assert(renderer != NULL);
    assert(renderer->mdc != NULL);

    HGDIOBJ brush;
    HPEN pen = NULL;

    if(thickness == 1)
    {
        SetDCPenColor(renderer->mdc, color);
    }
    else
    {
        pen = CreatePen(PS_SOLID, thickness, color);
        return_if(pen == NULL, "Failed to create pen", -1);
        SelectObject(renderer->mdc, pen);
    }

    brush = SelectObject(renderer->mdc, GetStockObject(NULL_BRUSH));

    if(rounding == 0)
    {
        Rectangle(renderer->mdc, x, y, x + width, y + height);
    }
    else
    {
        RoundRect(renderer->mdc, x, y, x + width, y + height, rounding, rounding);
    }

    SelectObject(renderer->mdc, brush);

    if(pen != NULL)
    {
        SelectObject(renderer->mdc, GetStockObject(DC_PEN));
        DeleteObject(pen);
    }

    return 0;
}

void render_filled_rect(Renderer *renderer, int16_t x, int16_t y, uint16_t width, uint16_t height, uint16_t rounding, COLORREF color)
{
    assert(renderer != NULL);
    assert(renderer->mdc != NULL);

    if(rounding == 0)
    {
        RECT rect;
        SetRect(&rect, x, y, x + width, y + height);
        SetBkColor(renderer->mdc, color);
        ExtTextOutW(renderer->mdc, 0, 0, ETO_OPAQUE, &rect, NULL, 0, NULL);
    }
    else
    {
        SetDCPenColor(renderer->mdc, color);
        SetDCBrushColor(renderer->mdc, color);
        RoundRect(renderer->mdc, x, y, x + width, y + height, rounding, rounding);
    }
}

int render_circle(Renderer *renderer, int16_t x, int16_t y, uint16_t width, uint16_t height, uint16_t thickness, COLORREF color)
{
    assert(renderer != NULL);
    assert(renderer->mdc != NULL);

    HPEN pen = NULL;
    if(thickness == 1)
    {
        SetDCPenColor(renderer->mdc, color);
    }
    else
    {
        pen = CreatePen(PS_SOLID, thickness, color);
        return_if(pen == NULL, "Failed to create pen", -1);
        SelectObject(renderer->mdc, pen);
    }

    SetDCBrushColor(renderer->mdc, OPAQUE);
    Ellipse(renderer->mdc, x, y, x + width, y + height);

    if(pen != NULL)
    {
        SelectObject(renderer->mdc, GetStockObject(DC_PEN));
        DeleteObject(pen);
    }

    return 0;
}

void render_filled_circle(Renderer *renderer, int16_t x, int16_t y, uint16_t width, uint16_t height, COLORREF color)
{
    assert(renderer != NULL);
    assert(renderer->mdc != NULL);

    SetDCBrushColor(renderer->mdc, color);
    SetDCPenColor(renderer->mdc, color);
    Ellipse(renderer->mdc, x, y, x + width, y + height);
}

int render_triangle(Renderer *renderer, int16_t x1, int16_t y1, int16_t x2, int16_t y2, int16_t x3, int16_t y3, uint16_t thickness, COLORREF color)
{
    assert(renderer != NULL);
    assert(renderer->mdc != NULL);

    HPEN pen = NULL;
    if(thickness == 1)
    {
        SetDCPenColor(renderer->mdc, color);
    }
    else
    {
        pen = CreatePen(PS_SOLID, thickness, color);
        return_if(pen == NULL, "Failed to create pen", -1);
        SelectObject(renderer->mdc, pen);
    }

    POINT points[4] = {
        {.x = x1, .y = y1},
        {.x = x2, .y = y2},
        {.x = x3, .y = y3},
        {.x = x1, .y = y1},
    };

    Polyline(renderer->mdc, points, 4);

    if(pen != NULL)
    {
        SelectObject(renderer->mdc, GetStockObject(DC_PEN));
        DeleteObject(pen);
    }

    return 0;
}

void render_filled_triangle(Renderer *renderer, int16_t x1, int16_t y1, int16_t x2, int16_t y2, int16_t x3, int16_t y3, COLORREF color)
{
    assert(renderer != NULL);
    assert(renderer->mdc != NULL);

    POINT points[3] = {
        {.x = x1, .y = y1},
        {.x = x2, .y = y2},
        {.x = x3, .y = y3},
    };

    SetDCPenColor(renderer->mdc, color);
    SetDCBrushColor(renderer->mdc, color);
    Polygon(renderer->mdc, points, 3);
}

int render_polygon(Renderer *renderer, short *points, int32_t row_count, uint16_t thickness, COLORREF color)
{
    assert(renderer != NULL);
    assert(renderer->mdc != NULL);

    HPEN pen = NULL;
    if(thickness == 1)
    {
        SetDCPenColor(renderer->mdc, color);
    }
    else
    {
        pen = CreatePen(PS_SOLID, thickness, color);
        return_if(pen == NULL, "Failed to create pen", -1);
        SelectObject(renderer->mdc, pen);
    }

    if(row_count > 0)
    {
        MoveToEx(renderer->mdc, points[0], points[1], NULL);

        for(int32_t i = 0; i < row_count; ++i)
        {
            LineTo(renderer->mdc, points[i * 2], points[i * 2 + 1]);
        }

        LineTo(renderer->mdc, points[0], points[1]);
    }

    if(pen != NULL)
    {
        SelectObject(renderer->mdc, GetStockObject(DC_PEN));
        DeleteObject(pen);
    }

    return 0;
}

void render_filled_polygon(Renderer *renderer, short *user_points, int32_t row_count, COLORREF color)
{
    assert(renderer != NULL);
    assert(renderer->mdc != NULL);

    SetDCBrushColor(renderer->mdc, color);
    SetDCPenColor(renderer->mdc, color);

    POINT points[64];

    int32_t i;
    for(i = 0; i < row_count && i < 64; ++i)
    {
        points[i].x = user_points[i * 2];
        points[i].y = user_points[i * 2 + 1];
    }

    Polygon(renderer->mdc, points, i);
}

int render_polyline(Renderer *renderer, short *points, int32_t row_count, uint16_t thickness, COLORREF color)
{
    assert(renderer != NULL);
    assert(renderer->mdc != NULL);

    HPEN pen = NULL;
    if(thickness == 1)
    {
        SetDCPenColor(renderer->mdc, color);
    }
    else
    {
        pen = CreatePen(PS_SOLID, thickness, color);
        return_if(pen == NULL, "Failed to create pen", -1);
        SelectObject(renderer->mdc, pen);
    }

    if(row_count > 0)
    {
        MoveToEx(renderer->mdc, points[0], points[1], NULL);
        for(int32_t i = 0; i < row_count; ++i)
        {
            LineTo(renderer->mdc, points[i * 2], points[i * 2 + 1]);
        }
    }

    if(pen != NULL)
    {
        SelectObject(renderer->mdc, GetStockObject(DC_PEN));
        DeleteObject(pen);
    }

    return 0;
}

int render_text(Renderer *renderer, Font *font, int16_t x, int16_t y, uint16_t width, uint16_t height, const char *text, int32_t len, COLORREF bg, COLORREF fg)
{
    (void)width;
    (void)height;

    assert(renderer != NULL);
    assert(renderer->mdc != NULL);
    
    if(text == NULL || font == NULL || len == 0)
    {
        return 0;
    }

    static wchar_t tmp_buff[50000];
    int utf16_len = MultiByteToWideChar(CP_UTF8, 0, text, len, NULL, 0);
    return_if(utf16_len > (int)(sizeof(tmp_buff) / sizeof(wchar_t)), "Not enough memory for utf-16 text", -1);
    return_if_win(MultiByteToWideChar(CP_UTF8, 0, text, len, tmp_buff, utf16_len) == 0, "Failed to translate utf-8 text to utf-16 text", -1);

    SetBkColor(renderer->mdc, bg);
    SetTextColor(renderer->mdc, fg);
    SelectObject(renderer->mdc, font->handle);
    ExtTextOutW(renderer->mdc, x, y, ETO_OPAQUE, NULL, tmp_buff, utf16_len, NULL);

    return 0;
}

int render_curve(Renderer *renderer, int16_t p1[2], int16_t p2[2], int16_t p3[2], int16_t p4[2], uint16_t thickness, COLORREF color)
{
    assert(renderer != NULL);
    assert(renderer->mdc != NULL);

    HPEN pen = NULL;
    if(thickness == 1)
    {
        SetDCPenColor(renderer->mdc, color);
    }
    else
    {
        pen = CreatePen(PS_SOLID, thickness, color);
        return_if(pen == NULL, "Failed to create pen", -1);
        SelectObject(renderer->mdc, pen);
    }

    POINT points[4] = {
        {.x = p1[0], .y = p1[1]},
        {.x = p2[0], .y = p2[1]},
        {.x = p3[0], .y = p3[1]},
        {.x = p4[0], .y = p4[1]}
    };

    SetDCBrushColor(renderer->mdc, OPAQUE);
    PolyBezier(renderer->mdc, points, 4);

    if(pen != NULL)
    {
        SelectObject(renderer->mdc, GetStockObject(DC_PEN));
        DeleteObject(pen);
    }

    return 0;
}

static inline void set_vertex_color(TRIVERTEX *vt, COLORREF color)
{
    vt->Red = (0xFF & color) << 8;
    vt->Green = (0xFF & (color >> 8)) << 8;
    vt->Blue = (0xFF & (color >> 16)) << 8;
    vt->Alpha = 0xFF << 8;
}

void render_multi_color_rect(Renderer *renderer, int16_t x, int16_t y, uint16_t width, uint16_t height, COLORREF left, COLORREF top, COLORREF right, COLORREF bottom)
{
    assert(renderer != NULL);
    assert(renderer->mdc != NULL);
    assert(renderer->wdc != NULL);

    BLENDFUNCTION alpha_function = {
        .BlendOp = AC_SRC_OVER,
        .BlendFlags = 0,
        .SourceConstantAlpha = 0,
        .AlphaFormat = AC_SRC_ALPHA
    };

    GRADIENT_TRIANGLE tris[2] = {
        {.Vertex1 = 0, .Vertex2 = 1, .Vertex3 = 2},
        {.Vertex1 = 2, .Vertex2 = 1, .Vertex3 = 3}
    };

    TRIVERTEX vt[4];

    vt[0].x = x;
    vt[0].y = y;
    set_vertex_color(&vt[0], left);

    vt[1].x = x + width;
    vt[1].y = y;
    set_vertex_color(&vt[1], top);

    vt[2].x = x;
    vt[2].y = y + height;
    set_vertex_color(&vt[2], right);

    vt[3].x = x + width;
    vt[3].y = y + height;
    set_vertex_color(&vt[3], bottom);

    GdiGradientFill(renderer->mdc, vt, 4, tris, 2, GRADIENT_FILL_TRIANGLE);
    AlphaBlend(renderer->wdc, x, y, x + width, y + height, renderer->mdc, x, y, x + width, y + height, alpha_function);
}

int render_image(Renderer *renderer, int16_t x, int16_t y, uint16_t width, uint16_t height, HBITMAP image)
{
    assert(renderer != NULL);
    assert(renderer->mdc != NULL);

    if(image == NULL)
    {
        return 0;
    }

    HDC bits = CreateCompatibleDC(renderer->mdc);
    return_if(bits == NULL, "Failed to create image dc", -1);

    BITMAP bitmap;
    GetObjectA(image, sizeof(BITMAP), &bitmap);
    SelectObject(bits, image);
    StretchBlt(renderer->mdc, x, y, width, height, bits, 0, 0, bitmap.bmWidth, bitmap.bmHeight, SRCCOPY);
    DeleteDC(bits);

    return 0;
}
