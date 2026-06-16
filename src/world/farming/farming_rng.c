#include "farming_rng.h"

// splitmix64 finalizer, same one we lean on everywhere. fold, stir, take the
// high bits.
static uint64_t mix64(uint64_t z) {
    z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ull;
    z = (z ^ (z >> 27)) * 0x94d049bb133111ebull;
    return z ^ (z >> 31);
}

uint32_t farming_hash3(int x, int y, int z, uint32_t seed) {
    uint64_t h = (uint64_t)(uint32_t)x * 0x9e3779b97f4a7c15ull;
    h ^= (uint64_t)(uint32_t)y * 0x85ebca6bull;
    h ^= (uint64_t)(uint32_t)z * 0xc2b2ae3d27d4eb4full;
    // fold the seed in twice, shifted, so adjacent seeds dont alias.
    h ^= (uint64_t)seed << 17;
    h ^= (uint64_t)seed >> 3;
    return (uint32_t)(mix64(h) >> 32);
}

float farming_hash_f01(int x, int y, int z, uint32_t seed) {
    // 24 mantissa bits, dodges the round-to-1 trap.
    return (farming_hash3(x, y, z, seed) >> 8) * (1.0f / 16777216.0f);
}

void farming_rng_seed(farming_rng *r, uint32_t seed) {
    // xorshift despises a zero state. never feed it one.
    r->s = mix64((uint64_t)seed * 0x2545f4914f6cdd1dull + 0x9e3779b9u);
    if (r->s == 0) r->s = 0xd1b54a32d192ed03ull;
}

uint32_t farming_rng_next(farming_rng *r) {
    uint64_t x = r->s;
    x ^= x >> 12;
    x ^= x << 25;
    x ^= x >> 27;
    r->s = x;
    return (uint32_t)((x * 0x2545f4914f6cdd1dull) >> 32);
}

float farming_rng_f01(farming_rng *r) {
    return (farming_rng_next(r) >> 8) * (1.0f / 16777216.0f);
}

int farming_rng_range(farming_rng *r, int lo, int hi) {
    if (hi <= lo) return lo;
    uint32_t span = (uint32_t)(hi - lo) + 1u;
    return lo + (int)(farming_rng_next(r) % span);
}

int farming_rng_chance(farming_rng *r, float p) {
    if (p <= 0.0f) return 0;
    if (p >= 1.0f) return 1;
    return farming_rng_f01(r) < p;
}
