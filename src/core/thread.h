#pragma once

#include <Windows.h>

typedef DWORD (*ThreadProc)(void *args);

int create_thread(ThreadProc proc, void *args);
