#include "erosion_noise.h"

#include <math.h>

// splitmix-ish integer mix. cheap, decent avalanche.
static uint32_t mix32(uint32_t h) {
    h ^= h >> 16;
    h *= 0x7feb352du;
    h ^= h >> 15;
    h *= 0x846ca68bu;
    h ^= h >> 16;
    return h;
}

uint32_t erosion_hash2(int x, int z, uint32_t seed) {
    uint32_t h = seed * 0x9e3779b1u;
    h ^= (uint32_t)x * 0x85ebca6bu;
    h = mix32(h);
    h ^= (uint32_t)z * 0xc2b2ae35u;
    return mix32(h);
}

float erosion_hash_f01(int x, int z, uint32_t seed) {
    // top 24 bits, scaled into [0,1). plenty for placement.
    return (erosion_hash2(x, z, seed) >> 8) * (1.0f / 16777216.0f);
}

uint32_t erosion_stream_seed(uint32_t seed, int droplet_index) {
    uint32_t s = mix32(seed ^ ((uint32_t)droplet_index * 0x9e3779b1u + 1u));
    // never hand back 0, the advance below would stall on it.
    return s ? s : 0x1234567u;
}

float erosion_stream_f01(uint32_t *state) {
    // simple lcg step then a final mix so low bits arent garbage.
    uint32_t s = *state;
    s = s * 1664525u + 1013904223u;
    *state = s;
    return (mix32(s) >> 8) * (1.0f / 16777216.0f);
}

static float fade(float t) {
    // quintic smoothstep, the perlin one.
    return t * t * t * (t * (t * 6.0f - 15.0f) + 10.0f);
}

static float lerpf(float a, float b, float t) {
    return a + (b - a) * t;
}

float erosion_value2(float x, float z, uint32_t seed) {
    int x0 = (int)floorf(x), z0 = (int)floorf(z);
    float fx = x - (float)x0, fz = z - (float)z0;

    // corner samples, recentred to [-1,1].
    float c00 = erosion_hash_f01(x0,     z0,     seed) * 2.0f - 1.0f;
    float c10 = erosion_hash_f01(x0 + 1, z0,     seed) * 2.0f - 1.0f;
    float c01 = erosion_hash_f01(x0,     z0 + 1, seed) * 2.0f - 1.0f;
    float c11 = erosion_hash_f01(x0 + 1, z0 + 1, seed) * 2.0f - 1.0f;

    float u = fade(fx), v = fade(fz);
    return lerpf(lerpf(c00, c10, u), lerpf(c01, c11, u), v);
}
