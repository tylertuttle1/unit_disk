#if !defined(VECTOR_H)

#include "basic.h"
#include "f32_math.h"

union v2
{
    struct
    {
        f32 x;
        f32 y;
    };

    f32 E[2];
};

union v2s
{
    struct
    {
        s32 x;
        s32 y;
    };

    s32 E[2];
};

union v3
{
    struct
    {
        f32 x;
        f32 y;
        f32 z;
    };

    struct
    {
        f32 r;
        f32 g;
        f32 b;
    };

    f32 E[3];
};

union v3s
{
    struct
    {
        s32 x;
        s32 y;
        s32 z;
    };

    struct
    {
        s32 r;
        s32 g;
        s32 b;
    };

    s32 E[3];
};

union v4
{
    struct
    {
        f32 x;
        f32 y;
        f32 z;
        f32 w;
    };

    struct
    {
        f32 r;
        f32 g;
        f32 b;
        f32 a;
    };

    struct
    {
        v3 xyz;
        f32 __unused4_0;
    };

    struct
    {
        v3 rgb;
        f32 __unused4_1;
    };

    f32 E[4];
};

union v4s
{
    struct
    {
        s32 x;
        s32 y;
        s32 z;
        s32 w;
    };

    struct
    {
        s32 r;
        s32 g;
        s32 b;
        s32 a;
    };

    struct
    {
        v3s xyz;
        s32 __unused4s_0;
    };

    struct
    {
        v3s rgb;
        s32 __unused4s_1;
    };

    s32 E[4];
};

static inline v2 make_v2(f32 x = 0.0f, f32 y = 0.0f);

static inline v2 operator+(v2 a, v2 b);
static inline v2 operator-(v2 a, v2 b);
static inline v2 operator*(f32 value, v2 a);
static inline v2 operator/(v2 a, f32 value);

static inline v2 operator+=(v2 a, v2 b);
static inline v2 operator-=(v2 a, v2 b);
static inline v2 operator*=(v2 a, f32 b);
static inline v2 operator/=(v2 a, f32 b);

static f32 dot(v2 a, v2 b);
static v2 hadamard(v2 a, v2 b);
static v2 perp(v2 a);
static v2 normalize(v2 a);
static f32 length(v2 a);
static f32 length_squared(v2 a);
static f32 distance(v2 a, v2 b);
static f32 distance_squared(v2 a, v2 b);

v2
operator+(v2 a, v2 b)
{
    v2 result;

    result.x = a.x + b.x;
    result.y = a.y + b.y;

    return result;
}

v2
operator-(v2 a, v2 b)
{
    v2 result;

    result.x = a.x - b.x;
    result.y = a.y - b.y;

    return result;
}

f32
length(v2 a)
{
    f32 result = sqrt_f32(a.x*a.x + a.y*a.y);
    return result;
}

f32
length_squared(v2 a)
{
    f32 result = a.x*a.x + a.y*a.y;
    return result;
}

f32
distance(v2 a, v2 b)
{
    f32 result = length(b - a);
    return result;
}

f32
distance_squared(v2 a, v2 b)
{
    f32 result = length_squared(b - a);
    return result;
}

internal f32
ccw(v2 a, v2 b, v2 c)
{
    f32 result = (b.x - a.x)*(c.y - a.y) - (b.y - a.y)*(c.x - a.x);
    return result;
}

internal bool
less_than(v2 a, v2 b, v2 c)
{
    if (a.y == c.y && b.y == c.y) {
        if (a.x >= c.x || b.x >= c.x) {
            return a.x > b.x;
        } else {
            return a.x < b.x;
        }
    }

    if (a.y > c.y && b.y < c.y)
        return true;

    if (a.y < c.y && b.y > c.y)
        return false;

    f32 det = ccw(c, b, a);
    if (det < 0.0f) {
        return true;
    } else if (det > 0.0f) {
        return false;
    } else {
        return distance_squared(a, c) > distance_squared(b, c);
    }
}

#define VECTOR_H
#endif
