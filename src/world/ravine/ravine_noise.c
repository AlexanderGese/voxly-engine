#include "ravine_noise.h"
#include "ravine_rand.h"

#include <math.h>

// lattice value at integer corner. reuse the rand hashes so we dont carry a
// second permutation table around. cast the floats through int corners.
static float corner(int xi, int zi, uint32_t seed) {
    // map [0,1) -> [-1,1]
    return ravine_hash_f01(xi, zi, seed) * 2.0f - 1.0f;
}

// smootherstep. perlin's 6t^5-15t^4+10t^3 — flat first and second derivative at
// the ends so the noise doesnt show grid creases.
static float fade(float t) {
    return t * t * t * (t * (t * 6.0f - 15.0f) + 10.0f);
}

float ravine_value2(float x, float z, uint32_t seed) {
    int x0 = (int)floorf(x), z0 = (int)floorf(z);
    int x1 = x0 + 1, z1 = z0 + 1;

    float tx = fade(x - (float)x0);
    float tz = fade(z - (float)z0);

    float c00 = corner(x0, z0, seed);
    float c10 = corner(x1, z0, seed);
    float c01 = corner(x0, z1, seed);
    float c11 = corner(x1, z1, seed);

    float a = c00 + tx * (c10 - c00);
    float b = c01 + tx * (c11 - c01);
    return a + tz * (b - a);
}

float ravine_fbm2(float x, float z, uint32_t seed,
                  int octaves, float lacunarity, float gain) {
    float sum = 0.0f, amp = 0.5f, freq = 1.0f, norm = 0.0f;
    if (octaves < 1) octaves = 1;
    for (int o = 0; o < octaves; o++) {
        // perturb the seed per octave so octaves dont share a lattice phase.
        sum  += amp * ravine_value2(x * freq, z * freq, seed + (uint32_t)o * 0x9e37u);
        norm += amp;
        amp  *= gain;
        freq *= lacunarity;
    }
    return norm > 0.0f ? sum / norm : 0.0f;
}

float ravine_ridge2(float x, float z, uint32_t seed, int octaves) {
    float sum = 0.0f, amp = 0.5f, freq = 1.0f, norm = 0.0f;
    if (octaves < 1) octaves = 1;
    for (int o = 0; o < octaves; o++) {
        float n = ravine_value2(x * freq, z * freq, seed + (uint32_t)o * 0x68bcu);
        // fold to a ridge: 1 - |n|, squared to sharpen the creases.
        n = 1.0f - fabsf(n);
        n *= n;
        sum  += amp * n;
        norm += amp;
        amp  *= 0.5f;
        freq *= 2.0f;
    }
    return norm > 0.0f ? sum / norm : 0.0f;
}
