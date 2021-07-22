#if !defined RNG_H

#include "basic.h"
#include <math.h>

struct RNG
{
    u64 state;
    u64 inc;
};

u32
random_u32(RNG *rng)
{
    u64 oldstate = rng->state;
    rng->state = oldstate * 6364136223846793005ULL + (rng->inc|1);
    u32 xorshifted = ((oldstate >> 18u) ^ oldstate) >> 27u;
    u32 rot = oldstate >> 59u;
    return (xorshifted >> rot) | (xorshifted << ((-rot) & 31));
}

f32
random_f32(RNG *rng)
{
    f32 result = ldexpf(random_u32(rng), -32);
    return result;
}

void
seed_rng(RNG *rng, u64 initstate, u64 initseq)
{
    rng->state = 0U;
    rng->inc = (initseq << 1u) | 1u;
    random_u32(rng);
    rng->state += initstate;
    random_u32(rng);
}

#define RNG_H
#endif
