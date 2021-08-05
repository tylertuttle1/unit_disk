// @IMPORTANT: WE HAVE TO INCLUDE STDLIB.H FIRST BEFORE INCLUDING BASIC.H
// SINCE FOR SOME REASON INCLUDING STDLIB.H WILL UNDEFINE MIN AND
// MAX AS MACROS SO YOU CAN'T USE THEM AS MACROS UNLESS YOU INCLUDE STDLIB.H
// FIRST BEFORE YOU DEFINE THEM

#include <stdlib.h>
#include <time.h>
#include <sys/mman.h>

#include "basic.h"
#include "platform.h"
#include "start.cpp"

struct LinuxMemoryBlock
{
    size_t size;
};

ALLOCATE_MEMORY(linux_allocate_memory)
{
    // void *result = malloc(size);

    size_t actual_size = size + sizeof(LinuxMemoryBlock);
    LinuxMemoryBlock *block = (LinuxMemoryBlock *) mmap(0, actual_size, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
    block->size = actual_size;

    void *result = ((char *) block + sizeof(LinuxMemoryBlock));

    return result;
}

FREE_MEMORY(linux_free_memory)
{
    // free(ptr);

    LinuxMemoryBlock *block = (LinuxMemoryBlock *) ptr - 1;
    munmap(block, block->size);
}

GET_CLOCK(linux_get_clock)
{
    u64 result;

    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
    result = 1000000000ull * ts.tv_sec + ts.tv_nsec;

    return result;
}

GET_CLOCK_FREQUENCY(linux_get_clock_frequency)
{
    u64 result = 1000000000ull;
    return result;
}

int
main(int argc, char **argv)
{
    allocate_memory = linux_allocate_memory;
    free_memory = linux_free_memory;
    get_clock = linux_get_clock;
    get_clock_frequency = linux_get_clock_frequency;

    int errorlevel = start();

    return errorlevel;
}
