#include "weathersim_rand.h"

// splitmix64 finalizer. fold the ints in, stir, take the high bits. same one
// oregen uses; copied rather than shared so weather and ore can never alias
// each other's state. cheap and well distributed enough for spawn rolls.
static uint64_t mix64(uint64_t z) {
    z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ull;
    z = (z ^ (z >> 27)) * 0x94d049bb133111ebull;
    return z ^ (z >> 31);
}

uint32_t weathersim_hash2(int x, int z, uint32_t seed) {
    uint64_t h = (uint64_t)(uint32_t)x * 0x9e3779b97f4a7c15ull;
    h ^= (uint64_t)(uint32_t)z * 0xc2b2ae3d27d4eb4full;
    h ^= (uint64_t)seed << 17;
    return (uint32_t)(mix64(h) >> 32);
}

uint32_t weathersim_hash3(int x, int y, int z, uint32_t seed) {
    uint64_t h = (uint64_t)(uint32_t)x * 0x9e3779b97f4a7c15ull;
    h ^= (uint64_t)(uint32_t)y * 0x85ebca6bull;
    h ^= (uint64_t)(uint32_t)z * 0xc2b2ae3d27d4eb4full;
    h ^= (uint64_t)seed << 11;
    return (uint32_t)(mix64(h) >> 32);
}

float weathersim_hash_f01(int x, int y, int z, uint32_t seed) {
    // 24 mantissa bits, dodges the rounding-to-1 trap.
    return (weathersim_hash3(x, y, z, seed) >> 8) * (1.0f / 16777216.0f);
}

uint32_t weathersim_seed_mix(uint32_t a, uint32_t b) {
    return (uint32_t)(mix64(((uint64_t)a << 32) | b) >> 32);
}

void weathersim_rng_seed(weathersim_rng *r, uint32_t seed) {
    // xorshift hates a zero state, so never hand it one.
    r->s = mix64((uint64_t)seed * 0x2545f4914f6cdd1dull + 0x9e3779b9u);
    if (r->s == 0) r->s = 0xa5a5a5a5deadc0deull;
}

uint32_t weathersim_rng_next(weathersim_rng *r) {
    uint64_t x = r->s;
    x ^= x >> 12;
    x ^= x << 25;
    x ^= x >> 27;
    r->s = x;
    return (uint32_t)((x * 0x2545f4914f6cdd1dull) >> 32);
}

int weathersim_rng_range(weathersim_rng *r, int lo, int hi) {
    if (hi <= lo) return lo;
    uint32_t span = (uint32_t)(hi - lo) + 1u;
    return lo + (int)(weathersim_rng_next(r) % span);
}

float weathersim_rng_f01(weathersim_rng *r) {
    return (weathersim_rng_next(r) >> 8) * (1.0f / 16777216.0f);
}

float weathersim_rng_frange(weathersim_rng *r, float lo, float hi) {
    return lo + (hi - lo) * weathersim_rng_f01(r);
}

int weathersim_rng_chance(weathersim_rng *r, float p) {
    if (p <= 0.0f) return 0;
    if (p >= 1.0f) return 1;
    return weathersim_rng_f01(r) < p;
}

float weathersim_rng_gauss(weathersim_rng *r) {
    // central-limit cheat: sum of 4 uniforms minus the mean, scaled so the
    // result lands roughly unit-variance. close enough for wind jitter.
    float s = 0.0f;
    for (int i = 0; i < 4; ++i) s += weathersim_rng_f01(r);
    return (s - 2.0f) * 0.8660254f; // sqrt(3/4) normalisation
}
