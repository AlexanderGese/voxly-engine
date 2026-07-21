#include "tonemap_curve.h"

#include <math.h>

// reference cpu implementations of the tonemap operators. the gpu shader in
// shaders/tonemap.frag mirrors these byte-for-byte (well, intent-for-intent);
// keeping a cpu copy lets the selftest poke at monotonicity and lets the
// auto-exposure code preview a luma without a readback.

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

// hable / uncharted2 filmic. the partial form; the full operator divides by
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
}

void tonemap_curve_init(tonemap_curve *c, int kind) {
    if (kind < 0 || kind >= TONEMAP_CURVE_COUNT)
        kind = TONEMAP_DEFAULT_CURVE;
    c->kind     = kind;
    c->white    = TONEMAP_DEFAULT_WHITE;
    c->exposure = 1.0f;
}

void tonemap_curve_set_exposure(tonemap_curve *c, float mult) {
    c->exposure = (mult < 0.0f) ? 0.0f : mult;
}

float tonemap_curve_apply1(const tonemap_curve *c, float x) {
    x *= c->exposure;
    if (x < 0.0f) x = 0.0f;   // negatives are nonsense post-exposure

    switch (c->kind) {
        case TONEMAP_CURVE_LINEAR:     return clampf(x, 0.0f, 1.0f);
        case TONEMAP_CURVE_REINHARD:   return tonemap_reinhard(x);
        case TONEMAP_CURVE_REINHARD_X: return tonemap_reinhard_ext(x, c->white);
        case TONEMAP_CURVE_ACES:       return tonemap_aces(x);
        case TONEMAP_CURVE_FILMIC:     return tonemap_filmic(x);
        default:                       return tonemap_aces(x);
    }
}

vec3 tonemap_curve_apply(const tonemap_curve *c, vec3 hdr) {
    return vec3_new(tonemap_curve_apply1(c, hdr.x),
                    tonemap_curve_apply1(c, hdr.y),
                    tonemap_curve_apply1(c, hdr.z));
}

const char *tonemap_curve_name(int kind) {
    switch (kind) {
        case TONEMAP_CURVE_LINEAR:     return "linear";
        case TONEMAP_CURVE_REINHARD:   return "reinhard";
        case TONEMAP_CURVE_REINHARD_X: return "reinhard-ext";
        case TONEMAP_CURVE_ACES:       return "aces";
        case TONEMAP_CURVE_FILMIC:     return "filmic";
        default:                       return "?";
    }
}
