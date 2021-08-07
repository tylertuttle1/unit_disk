#include "basic.h"
#include "platform.h"
#include "start.cpp"

#include "win32_defines.h"

ALLOCATE_MEMORY(win32_allocate_memory)
{
    void *result = VirtualAlloc(0, size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    return result;
}

FREE_MEMORY(win32_free_memory)
{
    VirtualFree(ptr, 0, MEM_RELEASE);
}

GET_CLOCK(win32_get_clock)
{
    LARGE_INTEGER result;
    QueryPerformanceCounter(&result);
    return result.QuadPart;
}

GET_CLOCK_FREQUENCY(win32_get_clock_frequency)
{
    LARGE_INTEGER result;
    QueryPerformanceFrequency(&result);
    return result.QuadPart;
}

int
main(int argc, char **argv)
{
    HANDLE stdout_handle = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD console_mode = 0;
    GetConsoleMode(stdout_handle, &console_mode);
    SetConsoleMode(stdout_handle, console_mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);

    allocate_memory = win32_allocate_memory;
    free_memory = win32_free_memory;
    get_clock = win32_get_clock;
    get_clock_frequency = win32_get_clock_frequency;

    int errorlevel = start();

    return errorlevel;
}

#if 0
void
mainCRTStartup()
{
    allocate_memory = win32_allocate_memory;
    free_memory = win32_free_memory;
    get_clock = win32_get_clock;
    get_clock_frequency = win32_get_clock_frequency;

    int errorlevel = start();

    ExitProcess(errorlevel);
}

extern "C" {
    int _fltused = 1234;

    #pragma function(memset)
    void *
    memset(void *dest, int value, size_t count)
    {
        __stosb(dest, value, count);
        return dest;
    }

    #pragma function(memcpy)
    void *
    memcpy(void *dest, void const *source, size_t count)
    {
        __movsb(dest, source, count);
        return dest;
    }
};
#endif
