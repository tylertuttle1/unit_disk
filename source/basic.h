#if !defined(BASIC_H)

// 
// @TODO
// 
// iswhitespace, isalpha, ishex, etc.
// string to int conversions
// defer macro? string type?
// round floats to ints, lsb, msb, popcount, etc?
// 

#include <stdint.h>
#include <stddef.h>

#if COMPILER_MSVC == 1
#include "win32_defines.h"
#endif

typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef float f32;
typedef double f64;

typedef uintptr_t umm;
typedef intptr_t smm;

#define arraycount(array) (sizeof(array) / sizeof((array)[0]))
// #define offsetof(type, member) ((size_t) &(((type *) 0)->member))
#define swap(a, b) do { auto swap = (a); (a) = (b); (b) = swap; } while (0)

// @NOTE: gcc's stdlib.h will undefine these macros for some reason?
#if 1
#define min(a, b) (((a) < (b)) ? (a) : (b))
#define max(a, b) (((a) > (b)) ? (a) : (b))

#define MIN(a, b)        (((a) < (b)) ? (a) : (b))
#define MIN3(a, b, c)    MIN((a), MIN((b), (c)))
#define MIN4(a, b, c, d) MIN(MIN((a), (b)), MIN((c), (d)))

#define MAX(a, b)        (((a) > (b)) ? (a) : (b))
#define MAX3(a, b, c)    MAX((a), MAX((b), (c)))
#define MAX4(a, b, c, d) MAX(MAX((a), (b)), MAX((c), (d)))
#else
template <typename T>
T min(T a, T b)
{
    return (a < b) ? a : b;
}

template <typename T>
T max(T a, T b)
{
    return (a > b) ? a : b;
}
#endif

// #define abs(a)    (((a) > 0) ? (a) : -(a))

#define CONCAT_(x,y) x##y
#define CONCAT(x,y) CONCAT_(x,y)
#define STRINGIFY_(s) #s
#define STRINGIFY(s) STRINGIFY_(s)

#define KB(n) ((n)*1024ull)
#define MB(n) (KB(n)*1024ull)
#define GB(n) (MB(n)*1024ull)
#define TB(n) (GB(n)*1024ull)

#define global     static
#define persistent static
#define internal   static

#define copy_array(dest, souce, count) copy_memory((dest), (source), (count)*sizeof((source)[0]))
#define clear_array(base, count) clear_memory((base), (count)*sizeof((base)[0]))
#define clear_struct(instance) clear_memory(&(instance), sizeof(instance))

internal void *
copy_memory(void *dest, void const *source, size_t count)
{
    u8 *dest_u8 = (u8 *) dest;
    u8 *source_u8 = (u8 *) source;

    while (count--) {
        *dest_u8++ = *source_u8++;
    }

    return dest;
}

internal void *
clear_memory(void *dest, size_t count)
{
    u8 *dest_u8 = (u8 *) dest;

    while (count--) {
        *dest_u8++ = 0;
    }

    return dest;
}

internal void *
set_memory(void *dest, u8 value, size_t count)
{
    u8 *dest_u8 = (u8 *) dest;

    while (count--) {
        *dest_u8++ = value;
    }

    return dest;
}

internal bool
memory_is_equal(void *a, void *b, size_t count)
{
    u8 *a8 = (u8 *) a;
    u8 *b8 = (u8 *) b;

    while (count--) {
        if (*a8++ != *b8++) return false;
    }

    return true;
}

u32
lsb(u32 value)
{
    // @TODO: is returning 0 when value == 0 really the best way to do this?
    // @NOTE: Knuth (TAOCP 4A p. 140) says we should return infinity

    u32 result = 0;

#if COMPILER_MSVC == 1
    DWORD dw = 0;
    _BitScanForward(&dw, value);
    result = (u32) dw;
#elif COMPILER_GCC == 1
    if (value) {
        result = __builtin_ctz(value);
    } else {
        result = 0;
    }
#endif

    return result;
}

u32
round_to_u32(f32 value)
{
    return (u32)(value + 0.5f);
}

s32
round_to_s32(f32 value)
{
    return (s32)(value + 0.5f);
}

f32
milliseconds(u64 start, u64 end, u64 freq)
{
    u64 counts_elapsed = end - start;
    f32 result = 1000.0f * (f32)(counts_elapsed) / (f32)(freq);
    return result;
}

#define BASIC_H
#endif
