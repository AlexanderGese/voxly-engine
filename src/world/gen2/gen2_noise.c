#include "gen2_noise.h"
#include <math.h>
uint32_t voxl_gen2_hash2(int x, int z, uint32_t seed) {
    uint32_t h = seed + 0x9e3779b9u;
    h ^= (uint32_t)x * 0x85ebca6bu;
    h = (h << 13) | (h >> 19);
    h ^= (uint32_t)z * 0xc2b2ae35u;
    h *= 0x27d4eb2fu;
    h ^= h >> 15;
    return h;
}

uint32_t voxl_gen2_hash3(int x, int y, int z, uint32_t seed) {
    uint32_t h = voxl_gen2_hash2(x, z, seed);
h ^= (uint32_t)y * 0x9e3779b9u;
h *= 0x85ebca6bu;
h ^= h >> 13;
return h;
}

float voxl_gen2_hash_f01(int x, int z, uint32_t seed) {
    // top 24 bits -> [0,1)
    uint32_t h = voxl_gen2_hash2(x, z, seed);
    return (float)(h >> 8) / (float)(1u << 24);
}

// --- value noise ---------------------------------------------------------

// per-cell pseudo random value in [-1,1]
static float cell_val(int xi, int zi, uint32_t seed) {
    uint32_t h = voxl_gen2_hash2(xi, zi, seed);
return (float)(h >> 8) / (float)(1u << 23) - 1.0f;
}

// smootherstep, ken perlin's 6t^5-15t^4+10t^3
static float fade(float t) {
    return t * t * t * (t * (t * 6.0f - 15.0f) + 10.0f);
}

static float lerpf(float a, float b, float t) {
    return a + (b - a) * t;
float sum = 0.0f, amp = 1.0f, freq = 1.0f, norm = 0.0f;
for (int o = 0;
o < octaves;
