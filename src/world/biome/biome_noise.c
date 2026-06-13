#include "biome_noise.h"

#include <math.h>

// --- hashing -------------------------------------------------------------

uint32_t biome_hash2(int x, int z, uint32_t seed) {
    uint32_t h = seed + 0x9e3779b9u;
    h ^= (uint32_t)x * 0x85ebca6bu;
    h = (h << 13) | (h >> 19);
    h ^= (uint32_t)z * 0xc2b2ae35u;
    h *= 0x27d4eb2fu;
    h ^= h >> 15;
    return h;
}

uint32_t biome_hash3(int x, int y, int z, uint32_t seed) {
    uint32_t h = biome_hash2(x, z, seed);
    h ^= (uint32_t)y * 0x9e3779b9u;
    h *= 0x85ebca6bu;
    h ^= h >> 13;
    return h;
}

float biome_hash01(int x, int z, uint32_t seed) {
    uint32_t h = biome_hash2(x, z, seed);
    return (float)(h >> 8) / (float)(1u << 24);   // top 24 bits -> [0,1)
}

// --- value noise ---------------------------------------------------------

// per-cell pseudo random value in [-1,1]
static float cell_val(int xi, int zi, uint32_t seed) {
    uint32_t h = biome_hash2(xi, zi, seed);
    return (float)(h >> 8) / (float)(1u << 23) - 1.0f;
}

// smootherstep, 6t^5-15t^4+10t^3
static float fade(float t) {
    return t * t * t * (t * (t * 6.0f - 15.0f) + 10.0f);
}

static float lerpf(float a, float b, float t) {
    return a + (b - a) * t;
}

float biome_value2(float x, float z, uint32_t seed) {
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

float biome_fbm2(float x, float z, uint32_t seed,
                 int octaves, float lacunarity, float gain) {
    if (octaves < 1) octaves = 1;
    float sum = 0.0f, amp = 1.0f, freq = 1.0f, norm = 0.0f;
    for (int o = 0; o < octaves; o++) {
        // offset seed per octave so layers dont line up
        sum  += amp * biome_value2(x * freq, z * freq, seed + (uint32_t)o * 1013u);
        norm += amp;
        amp  *= gain;
        freq *= lacunarity;
    }
    return (norm > 0.0f) ? sum / norm : 0.0f;
}

float biome_warp_fbm2(float x, float z, uint32_t seed, float warp, int octaves) {
    // cheap two-channel warp. offsets stolen so qx/qz dont share a field.
    float qx = biome_fbm2(x + 5.2f,  z + 1.3f,  seed ^ 0x111u, 2, 2.0f, 0.5f);
    float qz = biome_fbm2(x - 4.7f,  z + 8.9f,  seed ^ 0x222u, 2, 2.0f, 0.5f);
    return biome_fbm2(x + warp * qx, z + warp * qz, seed, octaves, 2.0f, 0.5f);
}
