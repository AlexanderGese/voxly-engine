#include "mineshaft_rand.h"

// splitmix64 finalizer, the workhorse. fold whatever ints we get into a u64,
// stir, return the well-mixed result. cheap and good enough for layout work.
static uint64_t mix64(uint64_t z) {
    z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ull;
    z = (z ^ (z >> 27)) * 0x94d049bb133111ebull;
    return z ^ (z >> 31);
}

uint32_t mineshaft_hash2(int x, int z, uint32_t seed) {
    uint64_t h = (uint64_t)(uint32_t)x * 0x9e3779b97f4a7c15ull;
    h ^= (uint64_t)(uint32_t)z * 0xc2b2ae3d27d4eb4full;
    h ^= (uint64_t)seed << 17;
    return (uint32_t)(mix64(h) >> 32);
}

uint32_t mineshaft_hash3(int x, int y, int z, uint32_t seed) {
    uint64_t h = (uint64_t)(uint32_t)x * 0x9e3779b97f4a7c15ull;
    h ^= (uint64_t)(uint32_t)y * 0x85ebca6bull;
    h ^= (uint64_t)(uint32_t)z * 0xc2b2ae3d27d4eb4full;
    h ^= (uint64_t)seed << 11;
    return (uint32_t)(mix64(h) >> 32);
}

float mineshaft_hash_f01(int x, int y, int z, uint32_t seed) {
    // top 24 bits, scaled. dodges the float rounding-to-exactly-1 trap.
    return (mineshaft_hash3(x, y, z, seed) >> 8) * (1.0f / 16777216.0f);
}

uint32_t mineshaft_seed_mix(uint32_t a, uint32_t b) {
    return (uint32_t)(mix64(((uint64_t)a << 32) | b) >> 32);
}

void mineshaft_rng_seed(mineshaft_rng *r, uint32_t seed) {
    // never let state land on zero, xorshift is dead there.
    r->s = mix64((uint64_t)seed * 0x2545f4914f6cdd1dull + 0x9e3779b9u);
    if (r->s == 0) r->s = 0x1d872b41c5e9f00dull;
}

uint32_t mineshaft_rng_next(mineshaft_rng *r) {
    // xorshift64*. local state only.
    uint64_t x = r->s;
    x ^= x >> 12;
    x ^= x << 25;
    x ^= x >> 27;
    r->s = x;
    return (uint32_t)((x * 0x2545f4914f6cdd1dull) >> 32);
}

int mineshaft_rng_range(mineshaft_rng *r, int lo, int hi) {
    if (hi <= lo) return lo;
    uint32_t span = (uint32_t)(hi - lo) + 1u;
    return lo + (int)(mineshaft_rng_next(r) % span);
}

float mineshaft_rng_f01(mineshaft_rng *r) {
    return (mineshaft_rng_next(r) >> 8) * (1.0f / 16777216.0f);
}

int mineshaft_rng_chance(mineshaft_rng *r, float p) {
    if (p <= 0.0f) return 0;
    if (p >= 1.0f) return 1;
    return mineshaft_rng_f01(r) < p;
}

void mineshaft_rng_shuffle(mineshaft_rng *r, int *a, int n) {
    // fisher-yates, back to front. classic, no bias if range() is unbiased-ish.
    for (int i = n - 1; i > 0; i--) {
        int j = mineshaft_rng_range(r, 0, i);
        int t = a[i]; a[i] = a[j]; a[j] = t;
    }
}
