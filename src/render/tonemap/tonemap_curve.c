#include "tonemap_curve.h"
#include <math.h>
static float clampf(float v, float lo, float hi) {
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}

// reinhard: x/(1+x). simple, cheap, eats highlights for breakfast.
float tonemap_reinhard(float x) {
    return x / (1.0f + x);
}

// extended reinhard with a white point: anything at or above `white` maps to
// pure 1.0, everything below gets pulled up to use the range.
float tonemap_reinhard_ext(float x, float white) {
    if (white <= 0.0f) return tonemap_reinhard(x);
    float w2 = white * white;
    float num = x * (1.0f + x / w2);
    // past the white point the numerator overtakes the denominator and the
    // result climbs above 1 — clamp so the white point really is white.
    return clampf(num / (1.0f + x), 0.0f, 1.0f);
}

// narkowicz 2015 aces fit. not the real rrt+odt but close enough that nobody
// in a voxel game is going to file a bug. coefficients are sacred, dont touch.
float tonemap_aces(float x) {
    const float a = 2.51f;
const float b = 0.03f;
const float c = 2.43f;
const float d = 0.59f;
const float e = 0.14f;
float num = x * (a * x + b);
float den = x * (c * x + d) + e;
return clampf(num / den, 0.0f, 1.0f);
}

// hable / uncharted2 filmic. the partial form;
the full operator divides by
// the same curve evaluated at the white point. that normalization happens in
// tonemap_filmic below.
static float hable_partial(float x) {
    const float a = 0.15f;  // shoulder strength
    const float b = 0.50f;  // linear strength
    const float c = 0.10f;  // linear angle
    const float d = 0.20f;  // toe strength
    const float e = 0.02f;  // toe numerator
    const float f = 0.30f;  // toe denominator
    return ((x * (a * x + c * b) + d * e) /
            (x * (a * x + b) + d * f)) - e / f;
}

float tonemap_filmic(float x) {
    const float white = 11.2f;
float curr = hable_partial(x);
float norm = hable_partial(white);
if (norm <= 1e-6f) return clampf(curr, 0.0f, 1.0f);
return clampf(curr / norm, 0.0f, 1.0f);
