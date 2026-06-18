#include "heightmap_noise.h"

#include <math.h>

// --- hashing -------------------------------------------------------------

// splitmix flavoured. decent avalanche, no table to carry around.
static uint32_t mix32(uint32_t h) {
    h ^= h >> 16;
    h *= 0x7feb352du;
    h ^= h >> 15;
    h *= 0x846ca68bu;
    h ^= h >> 16;
    return h;
}

uint32_t heightmap_hash2(int x, int z, uint32_t seed) {
    uint32_t h = seed * 0x9e3779b1u + 0x632be59bu;
    h ^= (uint32_t)x * 0x85ebca6bu;
    h = mix32(h);
    h ^= (uint32_t)z * 0xc2b2ae35u;
    return mix32(h);
}

float heightmap_hash01(int x, int z, uint32_t seed) {
    // top 24 bits scaled into [0,1). low bits of this hash are fine but the
    // top ones are free so why argue.
    return (heightmap_hash2(x, z, seed) >> 8) * (1.0f / 16777216.0f);
}

// --- value noise ---------------------------------------------------------

static float fade(float t) {
    // quintic smootherstep, the usual one
    return t * t * t * (t * (t * 6.0f - 15.0f) + 10.0f);
}

static float lerpf(float a, float b, float t) {
    return a + (b - a) * t;
}

// corner value recentred into [-1,1]
static float corner(int xi, int zi, uint32_t seed) {
    return heightmap_hash01(xi, zi, seed) * 2.0f - 1.0f;
}

float heightmap_value2(float x, float z, uint32_t seed) {
    float fx = floorf(x), fz = floorf(z);
    int xi = (int)fx, zi = (int)fz;
    float tx = fade(x - fx);
    float tz = fade(z - fz);

    float c00 = corner(xi,     zi,     seed);
    float c10 = corner(xi + 1, zi,     seed);
    float c01 = corner(xi,     zi + 1, seed);
    float c11 = corner(xi + 1, zi + 1, seed);

    float a = lerpf(c00, c10, tx);
    float b = lerpf(c01, c11, tx);
    return lerpf(a, b, tz);
}

// --- fbm -----------------------------------------------------------------

float heightmap_fbm2(float x, float z, uint32_t seed,
                     int octaves, float lacunarity, float gain) {
    if (octaves < 1) octaves = 1;
    float sum = 0.0f, amp = 1.0f, freq = 1.0f, norm = 0.0f;
    for (int o = 0; o < octaves; o++) {
        // shuffle the seed per octave so the layers dont stack on top of each
        // other and form visible terraces
        uint32_t os = seed + (uint32_t)o * 0x9e3779b9u;
        sum  += amp * heightmap_value2(x * freq, z * freq, os);
        norm += amp;
        amp  *= gain;
        freq *= lacunarity;
    }
    return (norm > 0.0f) ? sum / norm : 0.0f;
}

float heightmap_ridged2(float x, float z, uint32_t seed,
                        int octaves, float lacunarity, float gain) {
    if (octaves < 1) octaves = 1;
    float sum = 0.0f, amp = 1.0f, freq = 1.0f, norm = 0.0f;
    float prev = 1.0f;
    for (int o = 0; o < octaves; o++) {
        uint32_t os = seed + (uint32_t)o * 0x85ebca6bu;
        float n = heightmap_value2(x * freq, z * freq, os);
        n = 1.0f - fabsf(n);   // fold into ridges
        n *= n;                 // sharpen them
        n *= prev;              // higher octaves only bite where lower ones do
        prev = n;
        sum  += amp * n;
        norm += amp;
        amp  *= gain;
        freq *= lacunarity;
    }
    float r = (norm > 0.0f) ? sum / norm : 0.0f;
    return r * 2.0f - 1.0f;   // [0,1] -> [-1,1] to match the others
}

// --- domain warp ---------------------------------------------------------

float heightmap_warp2(float x, float z, uint32_t seed, float warp,
                      int octaves) {
    // two cheap offset fields. constants are arbitrary, just need them to not
    // share a lattice with the main sample or the warp does nothing.
    float qx = heightmap_fbm2(x + 5.2f, z + 1.3f, seed ^ 0x0a0a1111u, 2, 2.0f, 0.5f);
    float qz = heightmap_fbm2(x - 4.7f, z + 8.9f, seed ^ 0x0b0b2222u, 2, 2.0f, 0.5f);
    return heightmap_fbm2(x + warp * qx, z + warp * qz, seed, octaves, 2.0f, 0.5f);
}
