#include "ravine_rand.h"

// xorshift32 for the streamy stuff, splitmix32 for the stateless hashes.

void ravine_rng_seed(ravine_rng *r, uint32_t seed) {
    // never let the state be zero or xorshift gets stuck forever.
    r->s = seed ? seed : 0x9e3779b9u;
}

uint32_t ravine_rng_u32(ravine_rng *r) {
    uint32_t x = r->s;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    r->s = x ? x : 0x6d2b79f5u;
    return r->s;
}

float ravine_rng_f01(ravine_rng *r) {
    // top 24 bits -> [0,1). plenty of mantissa for our purposes.
    return (float)(ravine_rng_u32(r) >> 8) / (float)(1u << 24);
}

float ravine_rng_frange(ravine_rng *r, float lo, float hi) {
    return lo + (hi - lo) * ravine_rng_f01(r);
}

int ravine_rng_range(ravine_rng *r, int lo, int hi) {
    if (hi <= lo) return lo;
    uint32_t span = (uint32_t)(hi - lo + 1);
    return lo + (int)(ravine_rng_u32(r) % span);
}

// the standard splitmix32 finalizer. good avalanche, dirt cheap.
static uint32_t mix32(uint32_t x) {
    x += 0x9e3779b9u;
    x = (x ^ (x >> 16)) * 0x21f0aaadu;
    x = (x ^ (x >> 15)) * 0x735a2d97u;
    return x ^ (x >> 15);
}

uint32_t ravine_hash1(uint32_t v, uint32_t seed) {
    return mix32(v ^ (seed * 0x85ebca6bu));
}

uint32_t ravine_hash2(int x, int z, uint32_t seed) {
    uint32_t h = seed;
    h = mix32(h ^ (uint32_t)x);
    h = mix32(h ^ (uint32_t)z * 0x27d4eb2fu);
    return h;
}

float ravine_hash_f01(int x, int z, uint32_t seed) {
    return (float)(ravine_hash2(x, z, seed) >> 8) / (float)(1u << 24);
}
