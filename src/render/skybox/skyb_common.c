#include "skyb_common.h"

#include <math.h>

float skyb_clampf(float v, float lo, float hi) {
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}

float skyb_sat(float v) {
    return skyb_clampf(v, 0.0f, 1.0f);
}

float skyb_lerpf(float a, float b, float t) {
    return a + (b - a) * t;
}

float skyb_smooth(float e0, float e1, float x) {
    // tolerate reversed edges so callers can fade either direction
    if (e0 == e1) return x < e0 ? 0.0f : 1.0f;
    float t = (x - e0) / (e1 - e0);
    t = skyb_sat(t);
    return t * t * (3.0f - 2.0f * t);
}

float skyb_remap(float x, float a, float b, float c, float d) {
    if (a == b) return c;
    float t = (x - a) / (b - a);
    return c + (d - c) * t;
}

skyb_rgb skyb_mix(skyb_rgb a, skyb_rgb b, float t) {
    skyb_rgb r;
    r.x = a.x + (b.x - a.x) * t;
    r.y = a.y + (b.y - a.y) * t;
    r.z = a.z + (b.z - a.z) * t;
    return r;
}

skyb_rgb skyb_rgb_scale(skyb_rgb c, float s) {
    skyb_rgb r;
    r.x = skyb_sat(c.x * s);
    r.y = skyb_sat(c.y * s);
    r.z = skyb_sat(c.z * s);
    return r;
}

skyb_rgb skyb_tonemap(skyb_rgb c, float exposure) {
    // not a real tonemapper, just an exposure mul + gamma so bright sunset
    // horizons roll off instead of clipping to white.
    skyb_rgb r;
    float g = 1.0f / 2.2f;
    r.x = powf(skyb_sat(c.x * exposure), g);
    r.y = powf(skyb_sat(c.y * exposure), g);
    r.z = powf(skyb_sat(c.z * exposure), g);
    return r;
}

float skyb_wrap24(float h) {
    h = fmodf(h, 24.0f);
    if (h < 0.0f) h += 24.0f;
    return h;
}

float skyb_fract(float x) {
    return x - floorf(x);
}

vec3 skyb_dir_from_angles(float altitude, float azimuth) {
    float ca = cosf(altitude);
    vec3 d;
    d.x = ca * cosf(azimuth);
    d.y = sinf(altitude);
    d.z = ca * sinf(azimuth);
    return vec3_normalize(d);
}
