#include "skyb_dither.h"

#include <math.h>
#include <stdint.h>

// 8x8 bayer / ordered-dither threshold matrix, values 0..63. the recursive
// construction, just unrolled because it's tiny and i'm not building it at
// runtime.
static const int BAYER8[8][8] = {
    {  0, 32,  8, 40,  2, 34, 10, 42 },
    { 48, 16, 56, 24, 50, 18, 58, 26 },
    { 12, 44,  4, 36, 14, 46,  6, 38 },
    { 60, 28, 52, 20, 62, 30, 54, 22 },
    {  3, 35, 11, 43,  1, 33,  9, 41 },
    { 51, 19, 59, 27, 49, 17, 57, 25 },
    { 15, 47,  7, 39, 13, 45,  5, 37 },
    { 63, 31, 55, 23, 61, 29, 53, 21 },
};

float skyb_bayer8(int x, int y) {
    x &= (SKYB_BAYER_N - 1);
    y &= (SKYB_BAYER_N - 1);
    return (float)BAYER8[y][x] / 64.0f;
}

// cheap integer hash -> [0,1). good enough for stable per-cell jitter.
static float hash01(uint32_t h) {
    h ^= h >> 16; h *= 0x7feb352du;
    h ^= h >> 15; h *= 0x846ca68bu;
    h ^= h >> 16;
    return (h >> 8) * (1.0f / 16777216.0f);
}

float skyb_dither_tri(int cell, float amp) {
    // triangular PDF: sum of two uniforms minus one, gives a -1..1 triangle
    // which has flatter spectrum than uniform dither (less visible).
    uint32_t c = (uint32_t)cell * 2654435761u;
    float u0 = hash01(c);
    float u1 = hash01(c ^ 0x9e3779b9u);
    return (u0 + u1 - 1.0f) * amp;
}

skyb_rgb skyb_dither_color(skyb_rgb c, int cell, int bits) {
    if (bits < 1) bits = 8;
    float lsb = 1.0f / (float)((1 << bits) - 1);
    // ~1 LSB triangular nudge, slightly different cell per channel so the
    // channels don't move together (avoids tinted noise).
    skyb_rgb r;
    r.x = skyb_sat(c.x + skyb_dither_tri(cell * 3 + 0, lsb));
    r.y = skyb_sat(c.y + skyb_dither_tri(cell * 3 + 1, lsb));
    r.z = skyb_sat(c.z + skyb_dither_tri(cell * 3 + 2, lsb));
    return r;
}

skyb_rgb skyb_dither_quantize(skyb_rgb c, int cell, int bits) {
    if (bits < 1) bits = 8;
    skyb_rgb d = skyb_dither_color(c, cell, bits);
    float levels = (float)((1 << bits) - 1);
    skyb_rgb q;
    q.x = floorf(d.x * levels + 0.5f) / levels;
    q.y = floorf(d.y * levels + 0.5f) / levels;
    q.z = floorf(d.z * levels + 0.5f) / levels;
    return q;
}
