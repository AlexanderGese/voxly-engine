#include "mineshaft_rand.h"
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
if (r->s == 0) r->s = 0x1d872b41c5e9f00dull;
uint32_t span = (uint32_t)(hi - lo) + 1u;
return lo + (int)(mineshaft_rng_next(r) % span);
if (p >= 1.0f) return 1;
return mineshaft_rng_f01(r) < p;
