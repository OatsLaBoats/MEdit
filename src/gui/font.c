#include <assert.h>

#include "../core/utils.h"

#include "font.h"

int font_init(Font *font, const char *name, int32_t size)
{
    assert(font != NULL);
    assert(name != NULL);
    assert(size > 0);

    TEXTMETRICW metric;
    HDC hdc = NULL;
    HFONT hfont = NULL;

    hdc = CreateCompatibleDC(0);
    return_if(hdc == NULL, "Failed to create font dc", -1);

    hfont = CreateFontA(size, 
                        0, 0, 0, 
                        FW_NORMAL, 
                        FALSE, FALSE, FALSE, 
                        DEFAULT_CHARSET, 
                        OUT_DEFAULT_PRECIS, 
                        CLIP_DEFAULT_PRECIS, 
                        CLEARTYPE_QUALITY, 
                        DEFAULT_PITCH | FF_DONTCARE, 
                        name);
    jump_if(hfont == NULL, "Failed to load font", cleanup);
    
    SelectObject(hdc, hfont);
    GetTextMetricsW(hdc, &metric);

    font->height = metric.tmHeight;
    font->handle = hfont;
    font->dc = hdc;
    
    return 0;
    
    cleanup:
    if_valid(hfont != NULL, DeleteObject(hfont));
    if_valid(hdc != NULL, DeleteDC(hdc));

    return -1;
}

void font_destroy(Font *font)
{
    assert(font != NULL);
    assert(font->handle != NULL);
    assert(font->dc != NULL);

    if_valid(font->handle != NULL, DeleteObject(font->handle));
    if_valid(font->dc != NULL, DeleteDC(font->dc));
    clean_struct(font);
}

long font_get_text_width(Font *font, const Char8 *text, int len)
{
    assert(font != NULL);
    assert(font->dc != NULL);
    assert(text != NULL);

    static wchar_t tmp_buff[50000];
    int utf16_len = MultiByteToWideChar(CP_UTF8, 0, (char *)text, len, NULL, 0);
    return_if_s(utf16_len == 0, 0);
    return_if(utf16_len > (int)(sizeof(tmp_buff) / sizeof(wchar_t)), "Not enough memory for utf-16 text", -1);
    return_if_win(MultiByteToWideChar(CP_UTF8, 0, (char *)text, len, tmp_buff, utf16_len) == 0, "Failed to translate utf-8 text to utf-16 text", -1);

    SIZE size;
    GetTextExtentPoint32W(font->dc, tmp_buff, utf16_len, &size);

    return size.cx;
}
