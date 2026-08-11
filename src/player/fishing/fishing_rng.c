#include "fishing_rng.h"

// splitmix64. one multiply-xor chain per draw, no state beyond the counter.

void fishing_rng_seed(fishing_rng *r, uint64_t seed) {
    // avoid an all-zero state landing on a degenerate-ish stream.
    r->s = seed ? seed : 0x9E3779B97F4A7C15ull;
}

uint64_t fishing_rng_u64(fishing_rng *r) {
    uint64_t z = (r->s += 0x9E3779B97F4A7C15ull);
    z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ull;
    z = (z ^ (z >> 27)) * 0x94D049BB133111EBull;
    return z ^ (z >> 31);
}

float fishing_rng_float(fishing_rng *r) {
    // top 24 bits -> [0,1). matches the precision of a float mantissa.
    uint32_t bits = (uint32_t)(fishing_rng_u64(r) >> 40);
    return (float)bits / (float)(1u << 24);
}

float fishing_rng_frange(fishing_rng *r, float lo, float hi) {
    return lo + (hi - lo) * fishing_rng_float(r);
}

int fishing_rng_range(fishing_rng *r, int lo, int hi) {
    if (hi <= lo) return lo;
    uint64_t span = (uint64_t)(hi - lo) + 1;
    // modulo bias is negligible at these tiny spans, not worth a reject loop.
    return lo + (int)(fishing_rng_u64(r) % span);
}

int fishing_rng_chance(fishing_rng *r, float p) {
    if (p <= 0.0f) return 0;
    if (p >= 1.0f) return 1;
    return fishing_rng_float(r) < p;
}
