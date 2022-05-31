#pragma once

#include "app.h"

nk_plugin_filter get_plugin_filter(ValueType type);
int parse_user_input(ValueType type, Char8 *buffer, size_t buffer_size, void *value, size_t value_capacity, size_t *value_size);

