#include "rng.h"

void rng_init(rng *r, uint64_t seed) {
    if (!seed) seed = 0xdeadbeefcafebabeull;
    r->s = seed;
}

uint64_t rng_u64(rng *r) {
    uint64_t x = r->s;
    x ^= x >> 12;
    x ^= x << 25;
    x ^= x >> 27;
    r->s = x;
    return x * 0x2545F4914F6CDD1Dull;
}

uint32_t rng_u32(rng *r) {
    return (uint32_t)(rng_u64(r) >> 32);
}

float rng_float01(rng *r) {
    return (float)(rng_u32(r) >> 8) / (float)(1 << 24);
}

int rng_range(rng *r, int lo, int hi) {
    if (hi <= lo) return lo;
    return lo + (int)(rng_u32(r) % (uint32_t)(hi - lo + 1));
}

float rng_frange(rng *r, float lo, float hi) {
    return lo + (hi - lo) * rng_float01(r);
}
