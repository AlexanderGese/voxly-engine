#include "mob_rng.h"

void voxl_mob_rng_seed(voxl_mob_rng *r, uint32_t seed) {
    // avoid the degenerate all-zero state of xorshift.
    r->s = seed ? seed : 0x9e3779b9u;
}

uint32_t voxl_mob_rng_u32(voxl_mob_rng *r) {
    uint32_t x = r->s;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    r->s = x;
    return x;
}

float voxl_mob_rng_f01(voxl_mob_rng *r) {
    // top 24 bits -> [0,1). matches a 32-bit float mantissa nicely.
    return (float)(voxl_mob_rng_u32(r) >> 8) / 16777216.0f;
}

float voxl_mob_rng_frange(voxl_mob_rng *r, float lo, float hi) {
    return lo + (hi - lo) * voxl_mob_rng_f01(r);
}

int voxl_mob_rng_range(voxl_mob_rng *r, int lo, int hi) {
    if (hi <= lo) return lo;
    uint32_t span = (uint32_t)(hi - lo) + 1u;
    return lo + (int)(voxl_mob_rng_u32(r) % span);
}

int voxl_mob_rng_chance(voxl_mob_rng *r, float p) {
    return voxl_mob_rng_f01(r) < p ? 1 : 0;
}
