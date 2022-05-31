#pragma once

#include <Windows.h>

#include "../core/text.h"

typedef struct Font
{
    int32_t height;
    HFONT handle;
    HDC dc;
} Font;

int font_init(Font *font, const char *name, int32_t size);
void font_destroy(Font *font);
long font_get_text_width(Font *font, const Char8 *text, int len);
