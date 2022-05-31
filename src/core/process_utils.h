#pragma once

#include <Windows.h>
#include <stddef.h>
#include <stdint.h>

#define PROCESS_NAME_LEN 256

typedef struct ProcessArray
{
    int32_t len;
    DWORD pids[2048];
} ProcessArray;

typedef struct Process
{
    DWORD pid;
    HANDLE handle;
    char name[PROCESS_NAME_LEN];
} Process;

typedef struct Region
{
    uint8_t *address;
    size_t size;
} Region;

typedef struct RegionArray
{
    int32_t len;
    Region regions[20480];
} RegionArray;

int get_process_array(ProcessArray *array);
int get_process_name(char *buffer, DWORD size, DWORD pid);
int get_process_memory_usage(DWORD pid, size_t *usage);

int process_init(Process *process, DWORD pid);
void process_destroy(Process *process);
int process_read_memory(Process *process, void *address, void *buffer, size_t size);
int process_write_memory(Process *process, void *address, void *buffer, size_t size);
void process_get_regions(Process *process, RegionArray *array, DWORD filter);
