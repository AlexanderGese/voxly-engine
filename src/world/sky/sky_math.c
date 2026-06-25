#include "sky_math.h"

#include <math.h>

float voxl_sky_clampf(float v, float lo, float hi) {
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}

float voxl_sky_lerpf(float a, float b, float t) {
    return a + (b - a) * t;
}

float voxl_sky_remap(float x, float a, float b, float c, float d) {
    float denom = b - a;
    if (denom == 0.0f) return c;   // degenerate range, just pin to start
    float t = (x - a) / denom;
    return c + (d - c) * t;
}

float voxl_sky_smooth(float edge0, float edge1, float x) {
    float denom = edge1 - edge0;
    if (denom == 0.0f) return x < edge0 ? 0.0f : 1.0f;
    float t = voxl_sky_clampf((x - edge0) / denom, 0.0f, 1.0f);
    return t * t * (3.0f - 2.0f * t);
}

vec3 voxl_sky_mix3(vec3 a, vec3 b, float t) {
    vec3 r;
    r.x = a.x + (b.x - a.x) * t;
    r.y = a.y + (b.y - a.y) * t;
    r.z = a.z + (b.z - a.z) * t;
    return r;
}

vec4 voxl_sky_mix4(vec4 a, vec4 b, float t) {
    vec4 r;
    r.x = a.x + (b.x - a.x) * t;
    r.y = a.y + (b.y - a.y) * t;
    r.z = a.z + (b.z - a.z) * t;
    r.w = a.w + (b.w - a.w) * t;
    return r;
}

float voxl_sky_wrap24(float h) {
    // fmodf can return negative for negative input, so fix that up.
    h = fmodf(h, 24.0f);
    if (h < 0.0f) h += 24.0f;
    return h;
}

float voxl_sky_fract(float x) {
    return x - floorf(x);
}
