#include "sky_rand.h"

void voxl_sky_rand_seed(voxl_sky_rand *r, uint32_t seed) {
    r->s = seed ? seed : 0x9e3779b9u;   // golden-ratio fallback
}

uint32_t voxl_sky_rand_u32(voxl_sky_rand *r) {
    uint32_t x = r->s;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    r->s = x;
    return x;
}

float voxl_sky_rand_f(voxl_sky_rand *r) {
    // top 24 bits -> [0,1). divide by 2^24.
    return (voxl_sky_rand_u32(r) >> 8) / 16777216.0f;
}

float voxl_sky_rand_range(voxl_sky_rand *r, float lo, float hi) {
    return lo + (hi - lo) * voxl_sky_rand_f(r);
}

int voxl_sky_rand_int(voxl_sky_rand *r, int lo, int hi) {
    if (hi <= lo) return lo;
    uint32_t span = (uint32_t)(hi - lo + 1);
    return lo + (int)(voxl_sky_rand_u32(r) % span);
}
