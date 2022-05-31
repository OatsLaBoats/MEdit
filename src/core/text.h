#pragma once

#include <stdint.h>

typedef uint8_t Char8;
typedef uint16_t Char16;
typedef uint32_t Char32;

int convert_utf8_to_utf32(const Char8 *src, Char32 *dst, int dst_total_len, int *dst_len);
int convert_utf8_to_utf16(const Char8 *src, Char16 *dst, int dst_total_len, int *dst_len);

int convert_utf32_to_utf8(const Char32 *src, Char8 *dst, int dst_total_len, int *dst_len);
int convert_utf16_to_utf8(const Char16 *src, Char8 *dst, int dst_total_len, int *dst_len);
