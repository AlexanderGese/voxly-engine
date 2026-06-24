#include "rivers_rand.h"

// --- hashing -------------------------------------------------------------

uint32_t rivers_seed_mix(uint32_t a, uint32_t b) {
    // splitmix32 style finalizer on the two combined.
    uint32_t h = a ^ (b + 0x9e3779b9u + (a << 6) + (a >> 2));
    h ^= h >> 16;
    h *= 0x7feb352du;
    h ^= h >> 15;
    h *= 0x846ca68bu;
    h ^= h >> 16;
    return h;
}

uint32_t rivers_hash2(int x, int z, uint32_t seed) {
    uint32_t h = seed + 0x9e3779b9u;
    h ^= (uint32_t)x * 0x85ebca6bu; h = (h << 13) | (h >> 19);
    h ^= (uint32_t)z * 0xc2b2ae35u; h = (h << 11) | (h >> 21);
    h *= 0x165667b1u;
    h ^= h >> 15;
    return h;
}

uint32_t rivers_hash3(int x, int y, int z, uint32_t seed) {
    uint32_t h = rivers_hash2(x, z, seed);
    h ^= (uint32_t)y * 0x27d4eb2fu;
    h *= 0x9e3779b1u;
    h ^= h >> 15;
    return h;
}

float rivers_hash_f01(int x, int z, uint32_t seed) {
    uint32_t h = rivers_hash2(x, z, seed);
    // top 24 bits give an even spread without the low-bit banding.
    return (float)(h >> 8) / (float)(1u << 24);
}

// --- stateful rng --------------------------------------------------------
// xorshift64* with a splitmix seed expansion, same shape as math/rng but with
// its own type so it never aliases the world stream.

void rivers_rng_seed(rivers_rng *r, uint32_t seed) {
    uint64_t z = (uint64_t)seed * 0x9e3779b97f4a7c15ull + 0x9e3779b97f4a7c15ull;
    z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ull;
    z = (z ^ (z >> 27)) * 0x94d049bb133111ebull;
    z ^= z >> 31;
    // never let state be zero, xorshift dies there.
    r->s = z ? z : 0x1234567890abcdefull;
}

uint32_t rivers_rng_next(rivers_rng *r) {
    uint64_t x = r->s;
    x ^= x >> 12;
    x ^= x << 25;
    x ^= x >> 27;
    r->s = x;
    return (uint32_t)((x * 0x2545f4914f6cdd1dull) >> 32);
}

int rivers_rng_range(rivers_rng *r, int lo, int hi) {
    if (hi <= lo) return lo;
    uint32_t span = (uint32_t)(hi - lo + 1);
    return lo + (int)(rivers_rng_next(r) % span);
}

float rivers_rng_f01(rivers_rng *r) {
    return (float)(rivers_rng_next(r) >> 8) / (float)(1u << 24);
}

float rivers_rng_frange(rivers_rng *r, float lo, float hi) {
    return lo + (hi - lo) * rivers_rng_f01(r);
}

int rivers_rng_chance(rivers_rng *r, float p) {
    return rivers_rng_f01(r) < p ? 1 : 0;
}
