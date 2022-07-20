#include "process_utils.h"
#include "utils.h"

#include <Psapi.h>
#include <assert.h>

int get_process_array(ProcessArray *array)
{
    assert(array != NULL);

    DWORD bytes_used;
    return_if_win(!EnumProcesses(array->pids, sizeof(array->pids), &bytes_used), "Failed to get process list", -1);
    array->len = (int32_t)(bytes_used / sizeof(DWORD));
    
    return 0;
}

int get_process_name(char *buffer, DWORD size, DWORD pid)
{
    assert(buffer != NULL);

    HANDLE process = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);
    if(process == NULL)
    {
        DWORD error = GetLastError();
        
        // If the error is error code 5 then ignore it and return.
        return_if_s(error == 5, -1);

        log_error("Failed to open process with pid %lu (Error Code: %lu)", pid, error);
        return -1;
    }

    HMODULE base_module;
    DWORD needed;
    if(!EnumProcessModules(process, &base_module, sizeof(HMODULE), &needed))
    {
        log_win_error("Failed to read modules");
        CloseHandle(process);
        return -1;
    }

    if(!GetModuleBaseNameA(process, base_module, buffer, size))
    {
        log_win_error("Failed to get module name");
        CloseHandle(process);
        return -1;
    }

    CloseHandle(process);
    return 0;
}

int get_process_memory_usage(DWORD pid, size_t *usage)
{
    assert(usage != NULL);

    HANDLE process = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);
    if(process == NULL)
    {
        DWORD error = GetLastError();
        
        // If the error is error code 5 then ignore it and return.
        return_if_s(error == 5, -1);
        
        log_error("Failed to open process with pid %lu (Error Code: %lu)", pid, error);
        return -1;
    }
    
    PROCESS_MEMORY_COUNTERS pmc;
    if(!GetProcessMemoryInfo(process, &pmc, sizeof(PROCESS_MEMORY_COUNTERS)))
    {
        log_win_error("Failed to get process memory information");
        CloseHandle(process);
        return -1;
    }

    *usage = pmc.WorkingSetSize;

    CloseHandle(process);
    return 0;
}

int process_init(Process *process, DWORD pid)
{
    assert(process);

    process->pid = pid;
    return_if_s(get_process_name(process->name, sizeof(process->name), pid), -1);
    
    process->handle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
    return_if_win(process->handle == NULL, "Failed to open process", -1);

    return 0;
}

void process_destroy(Process *process)
{
    assert(process != NULL);
    if(process->handle != NULL) CloseHandle(process->handle);
    clean_struct(process);
}

int process_read_memory(Process *process, void *address, void *buffer, size_t size)
{
    assert(process != NULL);
    assert(process->handle != NULL);
    assert(buffer != NULL);
    assert(size != 0);

    SIZE_T read = 0;
    if(!ReadProcessMemory(process->handle, address, buffer, size, &read))
    {
        log_win_error("Failed to read memory at address %p size %llu bytes read %llu", address, size, read);
        return -1;
    }

    return 0;
}

int process_write_memory(Process *process, void *address, void *buffer, size_t size)
{
    assert(process != NULL);
    assert(process->handle != NULL);
    assert(buffer != NULL);
    assert(size != 0);

    SIZE_T written = 0;
    if(!WriteProcessMemory(process->handle, address, buffer, size, &written))
    {
        log_win_error("Failed to write memory at address %p size %llu bytes written %llu", address, size, written);
        return -1;
    }

    return 0;
}

void process_get_regions(Process *process, RegionArray *array, DWORD filter)
{
    assert(process != NULL);
    assert(process->handle != NULL);
    assert(array != NULL);

    MEMORY_BASIC_INFORMATION mbi;

    uint8_t *address = NULL;
    while(VirtualQueryEx(process->handle, address, &mbi, sizeof(mbi)))
    {
        // You can filter by page protect value here: mbi.Protect == PAGE_READWRITE etc.
        if(mbi.AllocationBase == NULL || mbi.State != MEM_COMMIT || !(mbi.Protect & filter))
        {
            address += mbi.RegionSize;
        }
        else
        {
            if(array->len >= c_array_capacity(array->regions))
            {
                log_warning("Not enough space in the buffer for memory regions");
                break;
            }

            address += mbi.RegionSize;
            array->regions[array->len].address = mbi.BaseAddress;
            array->regions[array->len].size = mbi.RegionSize;
            ++array->len;
        }
    }
}
