#include "text.h"
#include "utils.h"
#include <stdio.h>

#define check_bit(value, index) ((1 << (index - 1)) & value)

#define utf8_mid_char(value) (check_bit(value, 8) && !check_bit(value, 7))
#define utf8_1_byte(value) (!check_bit(value, 8))
#define utf8_2_byte(value) (check_bit(value, 8) && check_bit(value, 7) && !check_bit(value, 6))
#define utf8_3_byte(value) (check_bit(value, 8) && check_bit(value, 7) && check_bit(value, 6) && !check_bit(value, 5))
#define utf8_4_byte(value) (check_bit(value, 8) && check_bit(value, 7) && check_bit(value, 6) && check_bit(value, 5) && !check_bit(value, 4))

static const Char8 *decode_utf8_codepoint(const Char8 *src, Char32 *codepoint_ptr)
{
    Char32 codepoint = 0;
    Char8 b1 = 0;
    Char8 b2 = 0;
    Char8 b3 = 0;
    Char8 b4 = 0;

    b1 = *src++;
    if(utf8_1_byte(b1))
    {
        codepoint = b1;
    }
    else if(utf8_mid_char(b1))
    {
        return NULL;
    }
    else if(utf8_2_byte(b1))
    {
        b2 = *src++;
        return_if_s(!utf8_mid_char(b2), NULL);

        codepoint = ((b1 & 0x1F) << 6) | (b2 & 0x3F);
    }
    else if(utf8_3_byte(b1))
    {
        b2 = *src++;
        return_if_s(!utf8_mid_char(b2), NULL);

        b3 = *src++;
        return_if_s(!utf8_mid_char(b3), NULL);
        
        codepoint = ((b1 & 0xF) << 12) | ((b2 & 0x3F) << 6) | (b3 & 0x3F);
    }
    else if(utf8_4_byte(b1))
    {
        b2 = *src++;
        return_if_s(!utf8_mid_char(b2), NULL);

        b3 = *src++;
        return_if_s(!utf8_mid_char(b3), NULL);

        b4 = *src++;
        return_if_s(!utf8_mid_char(b4), NULL);
        
        codepoint = ((b1 & 0x7) << 18) | ((b2 & 0x3F) << 12) | ((b3 & 0x3F) << 6) | (b4 & 0x3F);
    }

    *codepoint_ptr = codepoint;
    return src;
}

static Char8 *encode_utf8_codepoint(Char8 *dst, int *dst_len, Char32 codepoint)
{
    *dst_len = 0;
    if(codepoint <= 0x7F)
    {
        dst[0] = (Char8)codepoint;
        *dst_len = 1;
    }
    else if(codepoint <= 0x7FF)
    {
        dst[0] = 0xC0 | (codepoint >> 6);
        dst[1] = 0x80 | (codepoint & 0x3F);
        *dst_len = 2;
    }
    else if(codepoint <= 0xFFFF)
    {
        dst[0] = 0xE0 | (codepoint >> 12);
        dst[1] = 0x80 | ((codepoint >> 6) & 0x3F);
        dst[2] = 0x80 | (codepoint & 0x3F);
        *dst_len = 3;
    }
    else if(codepoint <= 0x10FFFF)
    {
        dst[0] = 0xF0 | (codepoint >> 18);
        dst[1] = 0x80 | ((codepoint >> 12) & 0x3F);
        dst[2] = 0x80 | ((codepoint >> 6) & 0x3F);
        dst[3] = 0x80 | (codepoint & 0x3F);
        *dst_len = 4;
    }
    else
    {
        return NULL;
    }

    return dst + *dst_len;
}

static const Char16 *decode_utf16_codepoint(const Char16 *src, Char32 *codepoint)
{
    Char16 w1 = src[0];
    if(w1 < 0xD800 || w1 > 0xDFFF)
    {
        *codepoint = w1;
        return src + 1;
    }
    else
    {
        Char16 w2 = src[1];
        *codepoint = ((w1 - 0xD800) * 0x400) + (w2 - 0xDC00) + 0x10000;
        return src + 2;
    }
}

static Char16 *encode_utf16_codepoint(Char16 *dst, int *dst_len, Char32 codepoint)
{
    *dst_len = 0;
    if(codepoint < 0xD800 || (codepoint > 0xDFFF && codepoint <= 0xFFFF))
    {
        dst[0] = (Char16)codepoint;
        *dst_len = 1;
    }
    else
    {
        // Little endian encoding only
        uint32_t u = codepoint - 0x10000;
        Char16 w1 = (u >> 10) + 0xD800;
        Char16 w2 = (u & 0x3FF) + 0xDC00;
        dst[0] = w1;
        dst[1] = w2;
    }

    return dst + *dst_len;
}

int convert_utf8_to_utf32(const Char8 *src, Char32 *dst, int dst_total_len, int *dst_len)
{
    *dst_len = 0;
    int len = 0;
    while(*src != '\0' && len < dst_total_len - 1)
    {
        Char32 codepoint;
        src = decode_utf8_codepoint(src, &codepoint);       
        return_if_s(src == NULL, -1);
        dst[len++] = codepoint;
    }

    dst[len] = '\0';
    *dst_len = len;

    return 0;
}

int convert_utf8_to_utf16(const Char8 *src, Char16 *dst, int dst_total_len, int *dst_len)
{
    *dst_len = 0;
    int len = 0;
    while(*src != '\0' && len < dst_total_len - 3)
    {
        Char32 codepoint;
        src = decode_utf8_codepoint(src, &codepoint);       
        return_if_s(src == NULL, -1);

        int len16 = 0;
        dst = encode_utf16_codepoint(dst, &len16, codepoint);
        len += len16;
    }

    dst[len] = 0;
    *dst_len = len;

    return 0;
}

int convert_utf32_to_utf8(const Char32 *src, Char8 *dst, int dst_total_len, int *dst_len)
{
    *dst_len = 0;
    int len = 0;

    while(*src != '\0' && len < dst_total_len - 5) // -5 for an extra character and a '\0'
    {
        int len8 = 0;
        dst = encode_utf8_codepoint(dst, &len8, *src);
        return_if_s(dst == NULL, -1);
        len += len8;
        ++src;
    }

    dst[len] = 0;
    *dst_len = len;

    return 0;
}

int convert_utf16_to_utf8(const Char16 *src, Char8 *dst, int dst_total_len, int *dst_len)
{
    *dst_len = 0;
    int len = 0;

    while(*src != '\0' && len < dst_total_len - 5)
    {
        Char32 codepoint;
        src = decode_utf16_codepoint(src, &codepoint);

        int len8 = 0;
        dst = encode_utf8_codepoint(dst, &len8, codepoint);
        return_if_s(dst == NULL, -1);
        len += len8;
    }

    dst[len] = 0;
    *dst_len = len;

    return 0;
}
