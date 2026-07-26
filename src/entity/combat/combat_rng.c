#include "combat_rng.h"

// xorshift64*. cheap and good enough for picking crits.

void combat_rng_seed(combat_rng *r, uint64_t seed) {
    // avoid the all-zero state which would stick forever.
    r->s = seed ? seed : 0x9e3779b97f4a7c15ULL;
}

uint64_t combat_rng_next(combat_rng *r) {
    uint64_t x = r->s;
    x ^= x >> 12;
    x ^= x << 25;
    x ^= x >> 27;
    r->s = x;
    return x * 0x2545f4914f6cdd1dULL;
}

float combat_rng_unit(combat_rng *r) {
    // take the top 24 bits -> [0,1). 24 bits fits a float mantissa exactly.
    uint32_t bits = (uint32_t)(combat_rng_next(r) >> 40);
    return (float)bits / (float)(1u << 24);
}

bool combat_rng_chance(combat_rng *r, float p) {
    if (p <= 0.0f) return false;
    if (p >= 1.0f) return true;
    return combat_rng_unit(r) < p;
}

float combat_rng_range(combat_rng *r, float lo, float hi) {
    if (hi <= lo) return lo;
    return lo + combat_rng_unit(r) * (hi - lo);
}
