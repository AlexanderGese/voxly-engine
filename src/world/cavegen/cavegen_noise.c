#include "cavegen_noise.h"

#include <math.h>

// --- hashing -------------------------------------------------------------

uint32_t cavegen_hash1(uint32_t v, uint32_t seed) {
    uint32_t h = v + seed + 0x9e3779b9u;
    h ^= h >> 16;
    h *= 0x85ebca6bu;
    h ^= h >> 13;
    h *= 0xc2b2ae35u;
    h ^= h >> 16;
    return h;
}

uint32_t cavegen_hash3(int x, int y, int z, uint32_t seed) {
    uint32_t h = seed + 0x9e3779b9u;
    h ^= (uint32_t)x * 0x85ebca6bu; h = (h << 13) | (h >> 19);
    h ^= (uint32_t)y * 0xc2b2ae35u; h = (h << 11) | (h >> 21);
    h ^= (uint32_t)z * 0x27d4eb2fu;
    h *= 0x165667b1u;
    h ^= h >> 15;
    return h;
}

float cavegen_hash_f01(int x, int y, int z, uint32_t seed) {
    uint32_t h = cavegen_hash3(x, y, z, seed);
    return (float)(h >> 8) / (float)(1u << 24);
}

// --- value noise ---------------------------------------------------------

// per-cell value in [-1,1]
static float cell_val(int xi, int yi, int zi, uint32_t seed) {
    uint32_t h = cavegen_hash3(xi, yi, zi, seed);
    return (float)(h >> 8) / (float)(1u << 23) - 1.0f;
}

// smootherstep
static float fade(float t) {
    return t * t * t * (t * (t * 6.0f - 15.0f) + 10.0f);
}

static float lerpf(float a, float b, float t) {
    return a + (b - a) * t;
}

float cavegen_value3(float x, float y, float z, uint32_t seed) {
    float fx = floorf(x), fy = floorf(y), fz = floorf(z);
    int xi = (int)fx, yi = (int)fy, zi = (int)fz;
    float tx = fade(x - fx);
    float ty = fade(y - fy);
    float tz = fade(z - fz);

    // 8 corners. trilinear interp. unrolled because it reads cleaner than loops.
    float c000 = cell_val(xi,     yi,     zi,     seed);
    float c100 = cell_val(xi + 1, yi,     zi,     seed);
    float c010 = cell_val(xi,     yi + 1, zi,     seed);
    float c110 = cell_val(xi + 1, yi + 1, zi,     seed);
    float c001 = cell_val(xi,     yi,     zi + 1, seed);
    float c101 = cell_val(xi + 1, yi,     zi + 1, seed);
    float c011 = cell_val(xi,     yi + 1, zi + 1, seed);
    float c111 = cell_val(xi + 1, yi + 1, zi + 1, seed);

    float x00 = lerpf(c000, c100, tx);
    float x10 = lerpf(c010, c110, tx);
    float x01 = lerpf(c001, c101, tx);
    float x11 = lerpf(c011, c111, tx);

    float y0 = lerpf(x00, x10, ty);
    float y1 = lerpf(x01, x11, ty);

    return lerpf(y0, y1, tz);
}

float cavegen_fbm3(float x, float y, float z, uint32_t seed,
                   int octaves, float lacunarity, float gain) {
    float sum = 0.0f, amp = 1.0f, norm = 0.0f;
    float fx = x, fy = y, fz = z;
    for (int o = 0; o < octaves; o++) {
        // bump the seed each octave so layers dont rhyme with each other.
        sum  += amp * cavegen_value3(fx, fy, fz, seed + (uint32_t)o * 131u);
        norm += amp;
        amp  *= gain;
        fx *= lacunarity; fy *= lacunarity; fz *= lacunarity;
    }
    return norm > 0.0f ? sum / norm : 0.0f;
}

float cavegen_ridge3(float x, float y, float z, uint32_t seed, int octaves) {
    float sum = 0.0f, amp = 0.5f, norm = 0.0f;
    float fx = x, fy = y, fz = z;
    for (int o = 0; o < octaves; o++) {
        float n = cavegen_value3(fx, fy, fz, seed + (uint32_t)o * 911u);
        n = 1.0f - fabsf(n);   // fold to a ridge
        n *= n;                // sharpen
        sum  += amp * n;
        norm += amp;
        amp  *= 0.5f;
        fx *= 2.0f; fy *= 2.0f; fz *= 2.0f;
    }
    return norm > 0.0f ? sum / norm : 0.0f;
}
