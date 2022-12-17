#pragma once

#include "text.h"

void set_clipboard(const Char8 *text, int len);
Char8 *get_clipboard(int *result_size);
