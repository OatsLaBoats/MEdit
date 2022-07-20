#pragma once

#include <stdio.h>
#include <string.h>

#define kilobytes(v) (1024llu * v)
#define megabytes(v) (1024llu * kilobytes(v))

#define clean_struct(ptr) memset(ptr, 0, sizeof(*ptr))
#define c_array_capacity(array) (int)(sizeof(array) / sizeof(array[0]))

#define log_message(s, ...) fprintf(stdout, "\x1B[32mMESSAGE\x1B[0m::%s::" s "\n", __func__, __VA_ARGS__)
#define log_warning(s, ...) fprintf(stdout, "\x1B[33mWARNING\x1B[0m::%s::" s "\n", __func__, __VA_ARGS__)
#define log_error(s, ...) fprintf(stderr, "\x1B[31mERROR\x1B[0m::%s::" s "\n", __func__, __VA_ARGS__)
#define log_win_error(s, ...) fprintf(stderr, "\x1B[31mERROR\x1B[0m::%s::" s " (Error Code: %lu)\n", __func__, __VA_ARGS__, GetLastError())

// _s stands for silent and _win for windows

#define fn_exit_if_s(expression) \
if(expression) \
{ \
    return; \
}

#define fn_exit_if(expression, message) \
if(expression) \
{ \
    log_error(message); \
    return;\
}

#define fn_exit_if_win(expression, message) \
if(expression) \
{ \
    log_win_error(message); \
    return; \
}

#define return_if_s(expression, return_value) \
if(expression) \
{ \
    return return_value; \
}

#define return_if(expression, message, return_value) \
if(expression) \
{ \
    log_error(message); \
    return return_value; \
}

#define return_if_win(expression, message, return_value) \
if(expression) \
{ \
    log_win_error(message); \
    return return_value; \
}

#define break_if_s(expression) \
if(expression) \
{ \
    break; \
}

#define break_if(expression, message) \
if(expression) \
{ \
    log_error(message); \
    break; \
}

#define break_if_win(expression, message) \
if(expression) \
{ \
    log_win_error(message); \
    break; \
}

#define jump_if(expression, message, jump_id) \
if(expression) \
{ \
    log_error(message); \
    goto jump_id; \
}

#define jump_if_win(expression, message, jump_id) \
if(expression) \
{ \
    log_win_error(message); \
    goto jump_id; \
}

#define log_if(expression, message) \
if(expression) \
{ \
    log_error(message); \
}

#define log_if_win(expression, message) \
if(expression) \
{ \
    log_win_error(message); \
}
