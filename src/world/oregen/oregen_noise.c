#include "oregen_noise.h"
#include "oregen_rand.h"

// lattice value at an integer corner, in [-1,1].
static float corner(int x, int y, int z, uint32_t seed) {
    return oregen_hash_f01(x, y, z, seed) * 2.0f - 1.0f;
}

// quintic smootherstep, the perlin fade. flat slope at both ends so the
// trilinear blend doesnt show grid lines.
static float fade(float t) {
    return t * t * t * (t * (t * 6.0f - 15.0f) + 10.0f);
}

static float lerpf(float a, float b, float t) {
    return a + (b - a) * t;
}

float oregen_noise3(float x, float y, float z, uint32_t seed) {
    // floor to the lattice cell.
    int xi = (int)x - (x < 0.0f && x != (int)x);
    int yi = (int)y - (y < 0.0f && y != (int)y);
    int zi = (int)z - (z < 0.0f && z != (int)z);

    float fx = x - (float)xi;
    float fy = y - (float)yi;
    float fz = z - (float)zi;

    float u = fade(fx);
    float v = fade(fy);
    float w = fade(fz);

    // eight corners, trilinear blend.
    float c000 = corner(xi,     yi,     zi,     seed);
    float c100 = corner(xi + 1, yi,     zi,     seed);
    float c010 = corner(xi,     yi + 1, zi,     seed);
    float c110 = corner(xi + 1, yi + 1, zi,     seed);
    float c001 = corner(xi,     yi,     zi + 1, seed);
    float c101 = corner(xi + 1, yi,     zi + 1, seed);
    float c011 = corner(xi,     yi + 1, zi + 1, seed);
    float c111 = corner(xi + 1, yi + 1, zi + 1, seed);

    float x00 = lerpf(c000, c100, u);
    float x10 = lerpf(c010, c110, u);
    float x01 = lerpf(c001, c101, u);
    float x11 = lerpf(c011, c111, u);

    float y0 = lerpf(x00, x10, v);
    float y1 = lerpf(x01, x11, v);

    return lerpf(y0, y1, w);
}

float oregen_noise_fbm3(float x, float y, float z, uint32_t seed,
                        int octaves, float lacunarity, float gain) {
    if (octaves < 1) octaves = 1;
    float amp = 0.5f;
    float freq = 1.0f;
    float sum = 0.0f;
    float norm = 0.0f;

    for (int o = 0; o < octaves; o++) {
        // perturb the seed per octave so octaves dont share a lattice.
        uint32_t os = oregen_seed_mix(seed, (uint32_t)(o + 1) * 0x68bc21ebu);
        sum  += amp * oregen_noise3(x * freq, y * freq, z * freq, os);
        norm += amp;
        freq *= lacunarity;
        amp  *= gain;
    }
    return norm > 0.0f ? sum / norm : 0.0f;
}

float oregen_noise_turb3(float x, float y, float z, uint32_t seed, int octaves) {
    if (octaves < 1) octaves = 1;
    float amp = 0.5f;
    float freq = 1.0f;
    float sum = 0.0f;

    for (int o = 0; o < octaves; o++) {
        uint32_t os = oregen_seed_mix(seed, (uint32_t)(o + 7) * 0x2c1b3c6du);
        float n = oregen_noise3(x * freq, y * freq, z * freq, os);
        sum  += amp * (n < 0.0f ? -n : n);
        freq *= 2.0f;
        amp  *= 0.5f;
    }
    return sum;
}

float oregen_noise_surface(int cx, int cy, int cz, int dx, int dy, int dz,
                           float amp, uint32_t seed) {
    // sample the field at the offset world position. scale down so the warp
    // wavelength is a few blocks, which is the right lumpiness for ore.
    float s = 0.35f;
    float n = oregen_noise_fbm3((float)(cx + dx) * s,
                                (float)(cy + dy) * s,
                                (float)(cz + dz) * s,
                                seed ^ 0x73af1c29u, 2, 2.0f, 0.5f);
    return n * amp;
}
