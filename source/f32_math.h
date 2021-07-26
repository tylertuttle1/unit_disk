#if !defined(F32_MATH_H)

#define USE_SSE2
#include "sse_mathfun.h"

f32
floor_f32(f32 value)
{
    f32 result = _mm_cvtss_f32(_mm_floor_ps(_mm_set_ss(value)));
    return result;
}

f32
ceil_f32(f32 value)
{
    f32 result = _mm_cvtss_f32(_mm_ceil_ps(_mm_set_ss(value)));
    return result;
}

f32
sqrt_f32(f32 value)
{
    f32 result = _mm_cvtss_f32(_mm_sqrt_ss(_mm_set_ss(value)));
    return result;
}

float
sin_f32(float value)
{
    float result = _mm_cvtss_f32(sin_ps(_mm_set_ss(value)));
    return result;
}

#define F32_MATH_H
#endif
