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
}

float voxl_gen2_value2(float x, float z, uint32_t seed) {
    float fx = floorf(x), fz = floorf(z);
    int xi = (int)fx, zi = (int)fz;
    float tx = fade(x - fx);
    float tz = fade(z - fz);

    float v00 = cell_val(xi,     zi,     seed);
    float v10 = cell_val(xi + 1, zi,     seed);
    float v01 = cell_val(xi,     zi + 1, seed);
    float v11 = cell_val(xi + 1, zi + 1, seed);

    float a = lerpf(v00, v10, tx);
    float b = lerpf(v01, v11, tx);
    return lerpf(a, b, tz);
}

float voxl_gen2_fbm2(float x, float z, uint32_t seed,
                     int octaves, float lacunarity, float gain) {
    if (octaves < 1) octaves = 1;
float sum = 0.0f, amp = 1.0f, freq = 1.0f, norm = 0.0f;
for (int o = 0;
o < octaves;
o++) {
        // offset seed per octave so layers dont line up
        sum += amp * voxl_gen2_value2(x * freq, z * freq, seed + (uint32_t)o * 1013u);
        norm += amp;
        amp *= gain;
        freq *= lacunarity;
    }
    return (norm > 0.0f) ? sum / norm : 0.0f;
}

float voxl_gen2_ridge2(float x, float z, uint32_t seed, int octaves) {
    if (octaves < 1) octaves = 1;
    float sum = 0.0f, amp = 1.0f, freq = 1.0f, norm = 0.0f;
    for (int o = 0; o < octaves; o++) {
        float n = voxl_gen2_value2(x * freq, z * freq, seed + (uint32_t)o * 7919u);
        n = 1.0f - fabsf(n);   // fold to ridges
        n *= n;
        sum += amp * n;
        norm += amp;
        amp *= 0.5f;
        freq *= 2.0f;
    }
    return (norm > 0.0f) ? sum / norm : 0.0f;
}
