#include "stronghold_rand.h"

// splitmix64 finalizer. fold ints into a u64, stir, return upper bits.
static uint64_t mix64(uint64_t z) {
    z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ull;
    z = (z ^ (z >> 27)) * 0x94d049bb133111ebull;
    return z ^ (z >> 31);
}

uint32_t stronghold_hash2(int x, int z, uint32_t seed) {
    uint64_t h = (uint64_t)(uint32_t)x * 0x9e3779b97f4a7c15ull;
    h ^= (uint64_t)(uint32_t)z * 0xc2b2ae3d27d4eb4full;
    h ^= (uint64_t)seed << 17;
    return (uint32_t)(mix64(h) >> 32);
}

uint32_t stronghold_hash3(int x, int y, int z, uint32_t seed) {
    uint64_t h = (uint64_t)(uint32_t)x * 0x9e3779b97f4a7c15ull;
    h ^= (uint64_t)(uint32_t)y * 0x85ebca6bull;
    h ^= (uint64_t)(uint32_t)z * 0xc2b2ae3d27d4eb4full;
    h ^= (uint64_t)seed << 11;
    return (uint32_t)(mix64(h) >> 32);
}

float stronghold_hash_f01(int x, int z, uint32_t seed) {
    return (stronghold_hash2(x, z, seed) >> 8) * (1.0f / 16777216.0f);
}

uint32_t stronghold_seed_mix(uint32_t a, uint32_t b) {
    return (uint32_t)(mix64(((uint64_t)a << 32) | b) >> 32);
}

void stronghold_rng_seed(stronghold_rng *r, uint32_t seed) {
    // never let state be zero, xorshift dies there.
    r->s = mix64((uint64_t)seed * 0x2545f4914f6cdd1dull + 0x9e37u);
    if (r->s == 0) r->s = 0xdeadbeefcafebabeull;
}

uint32_t stronghold_rng_next(stronghold_rng *r) {
    // xorshift64*. same family the engine rng uses, local state.
    uint64_t x = r->s;
    x ^= x >> 12;
    x ^= x << 25;
    x ^= x >> 27;
    r->s = x;
    return (uint32_t)((x * 0x2545f4914f6cdd1dull) >> 32);
}

int stronghold_rng_range(stronghold_rng *r, int lo, int hi) {
    if (hi <= lo) return lo;
    uint32_t span = (uint32_t)(hi - lo) + 1u;
    return lo + (int)(stronghold_rng_next(r) % span);
}

float stronghold_rng_f01(stronghold_rng *r) {
    return (stronghold_rng_next(r) >> 8) * (1.0f / 16777216.0f);
}

int stronghold_rng_chance(stronghold_rng *r, float p) {
    if (p <= 0.0f) return 0;
    if (p >= 1.0f) return 1;
    return stronghold_rng_f01(r) < p;
}

int stronghold_rng_weighted(stronghold_rng *r, const int *weights, int n) {
    int total = 0;
    for (int i = 0; i < n; i++) total += weights[i] > 0 ? weights[i] : 0;
    if (total <= 0) return 0;
    int roll = (int)(stronghold_rng_next(r) % (uint32_t)total);
    for (int i = 0; i < n; i++) {
        int w = weights[i] > 0 ? weights[i] : 0;
        if (roll < w) return i;
        roll -= w;
    }
    return n - 1;   // float-free, cant fall through, but be safe
}
