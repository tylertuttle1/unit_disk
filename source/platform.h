#if !defined(PLATFORM_H)

#include "basic.h"

#define ALLOCATE_MEMORY(name) void *name(size_t size)
typedef ALLOCATE_MEMORY(allocate_memory_t);

#define FREE_MEMORY(name) void name(void *ptr)
typedef FREE_MEMORY(free_memory_t);

#define GET_CLOCK(name) u64 name(void)
typedef GET_CLOCK(get_clock_t);

#define GET_CLOCK_FREQUENCY(name) u64 name(void)
typedef GET_CLOCK_FREQUENCY(get_clock_frequency_t);

global allocate_memory_t *allocate_memory;
global free_memory_t *free_memory;
global get_clock_t *get_clock;
global get_clock_frequency_t *get_clock_frequency;

#define PLATFORM_H
#endif
