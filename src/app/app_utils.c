#include "../core/utils.h"

#include "app_utils.h"
#include <stdint.h>
#include <stdlib.h>

nk_plugin_filter get_plugin_filter(ValueType type)
{
    switch(type)
    {
        case F32:
        case F64: return nk_filter_float;

        case UTF8: 
        case UTF16:
        case UTF32: return nk_filter_default;

        default: return nk_filter_decimal;
    }
}

int parse_user_input(ValueType type, Char8 *buffer, size_t buffer_size, void *value, size_t value_capacity, size_t *value_size)
{
    errno = 0;
    Char8 *end;

    switch(type)
    {
        case I8:
        {
            *value_size = sizeof(int8_t);
            int8_t v = (int8_t)strtol((char *)buffer, (char **)&end, 10);
            return_if(errno == ERANGE, "Invalid Input", -1);
            memcpy(value, &v, *value_size);
        } break;

        case U8:
        {
            *value_size = sizeof(uint8_t);
            uint8_t v = (uint8_t)strtoul((char *)buffer, (char **)&end, 10);
            return_if(errno == ERANGE, "Invalid Input", -1);
            memcpy(value, &v, *value_size);
        } break;

        case I16:
        {
            *value_size = sizeof(int16_t);
            int16_t v = (int16_t)strtol((char *)buffer, (char **)&end, 10);
            return_if(errno == ERANGE, "Invalid Input", -1);
            memcpy(value, &v, *value_size);
        } break;

        case U16:
        {
            *value_size = sizeof(uint16_t);
            uint16_t v = (uint16_t)strtoul((char *)buffer, (char **)&end, 10);
            return_if(errno == ERANGE, "Invalid Input", -1);
            memcpy(value, &v, *value_size);
        } break;

        case I32:
        {
            *value_size = sizeof(int32_t);
            int32_t v = (int32_t)strtol((char *)buffer, (char **)&end, 10);
            return_if(errno == ERANGE, "Invalid Input", -1);
            memcpy(value, &v, *value_size);
        } break;
        
        case U32:
        {
            *value_size = sizeof(uint32_t);
            uint32_t v = (uint32_t)strtoul((char *)buffer, (char **)&end, 10);
            return_if(errno == ERANGE, "Invalid Input", -1);
            memcpy(value, &v, *value_size);
        } break;

        case I64:
        {
            *value_size = sizeof(int64_t);
            int64_t v = strtoll((char *)buffer, (char **)&end, 10);
            return_if(errno == ERANGE, "Invalid Input", -1);
            memcpy(value, &v, *value_size);
        } break;

        case U64:
        {
            *value_size = sizeof(uint64_t);
            uint64_t v = strtoull((char *)buffer, (char **)&end, 10);
            return_if(errno == ERANGE, "Invalid Input", -1);
            memcpy(value, &v, *value_size);
        } break;

        case F32:
        {
            *value_size = sizeof(float);
            float v = strtof((char *)buffer, (char **)&end);
            return_if(errno == ERANGE, "Invalid Input", -1);
            memcpy(value, &v, *value_size);
        } break;

        case F64:
        {
            *value_size = sizeof(double);
            double v = strtod((char *)buffer, (char **)&end);
            return_if(errno == ERANGE, "Invalid Input", -1);
            memcpy(value, &v, *value_size);
        } break;

        case UTF8:
        {
            memcpy(value, buffer, buffer_size);
            *value_size = buffer_size;
        } break;

        case UTF16:
        {
            int len = 0;
            convert_utf8_to_utf16(buffer, (Char16 *)value, (int)(value_capacity / sizeof(Char16)), &len);
            *value_size = len * sizeof(Char16);
        } break;

        case UTF32:
        {
            int len = 0;
            convert_utf8_to_utf32(buffer, (Char32 *)value, (int)(value_capacity / sizeof(Char32)), &len);
            *value_size = len * sizeof(Char32);
        } break;
    }

    return 0;
}

