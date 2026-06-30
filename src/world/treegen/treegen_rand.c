#include "treegen_rand.h"

// splitmix32 avalanche. cheap, well-mixed enough for placement decisions.
static uint32_t mix32(uint32_t x) {
    x += 0x9e3779b9u;
    x = (x ^ (x >> 16)) * 0x85ebca6bu;
    x = (x ^ (x >> 13)) * 0xc2b2ae35u;
    return x ^ (x >> 16);
}

uint32_t treegen_hash2(int x, int z, uint32_t seed) {
    uint32_t h = seed;
    h = mix32(h ^ (uint32_t)(x * 0x1f1f1f1f));
    h = mix32(h ^ (uint32_t)(z * 0x27d4eb2f));
    return h;
}

uint32_t treegen_hash3(int x, int y, int z, uint32_t seed) {
    uint32_t h = treegen_hash2(x, z, seed);
    h = mix32(h ^ (uint32_t)(y * 0x165667b1));
    return h;
}

float treegen_hash_f01(int x, int z, uint32_t seed) {
    return (float)(treegen_hash2(x, z, seed) >> 8) / (float)(1 << 24);
}

uint32_t treegen_seed_mix(uint32_t a, uint32_t b) {
    return mix32(a ^ (mix32(b) + 0x9e3779b9u));
}

void treegen_rng_seed(treegen_rng *r, uint32_t seed) {
    // spread the 32-bit seed across the 64-bit state; never let it be zero.
    uint64_t s = ((uint64_t)mix32(seed) << 32) | (uint64_t)mix32(seed ^ 0x5bd1e995u);
    if (!s) s = 0xa0761d6478bd642full;
    r->s = s;
}

uint32_t treegen_rng_next(treegen_rng *r) {
    uint64_t x = r->s;
    x ^= x >> 12;
    x ^= x << 25;
    x ^= x >> 27;
    r->s = x;
    return (uint32_t)((x * 0x2545F4914F6CDD1Dull) >> 32);
}

int treegen_rng_range(treegen_rng *r, int lo, int hi) {
    if (hi <= lo) return lo;
    return lo + (int)(treegen_rng_next(r) % (uint32_t)(hi - lo + 1));
}

float treegen_rng_f01(treegen_rng *r) {
    return (float)(treegen_rng_next(r) >> 8) / (float)(1 << 24);
}

float treegen_rng_jitter(treegen_rng *r, float mag) {
    return (treegen_rng_f01(r) * 2.0f - 1.0f) * mag;
}

int treegen_rng_chance(treegen_rng *r, float p) {
    if (p <= 0.0f) return 0;
    if (p >= 1.0f) return 1;
    return treegen_rng_f01(r) < p;
}
