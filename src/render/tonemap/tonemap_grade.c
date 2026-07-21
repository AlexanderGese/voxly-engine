#include "tonemap_grade.h"

#include <math.h>

static float clampf(float v, float lo, float hi) {
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}

static vec3 clamp01(vec3 c) {
    return vec3_new(clampf(c.x, 0.0f, 1.0f),
                    clampf(c.y, 0.0f, 1.0f),
                    clampf(c.z, 0.0f, 1.0f));
}

void tonemap_grade_init(tonemap_grade *g) {
    g->contrast    = TONEMAP_DEFAULT_CONTRAST;
    g->saturation  = TONEMAP_DEFAULT_SATURATION;
    g->temperature = TONEMAP_DEFAULT_TEMP;
    g->tint        = TONEMAP_DEFAULT_TINT;
    g->lift  = VEC3_ZERO;
    g->gamma = vec3_new(1.0f, 1.0f, 1.0f);
    g->gain  = vec3_new(1.0f, 1.0f, 1.0f);
}

void tonemap_grade_sanitize(tonemap_grade *g) {
    g->contrast    = clampf(g->contrast,    0.0f, 4.0f);
    g->saturation  = clampf(g->saturation,  0.0f, 4.0f);
    g->temperature = clampf(g->temperature, -1.0f, 1.0f);
    g->tint        = clampf(g->tint,        -1.0f, 1.0f);

    g->lift  = vec3_new(clampf(g->lift.x, -1.0f, 1.0f),
                        clampf(g->lift.y, -1.0f, 1.0f),
                        clampf(g->lift.z, -1.0f, 1.0f));
    // gamma exponent of zero would blow up; keep it positive.
    g->gamma = vec3_new(clampf(g->gamma.x, 0.05f, 8.0f),
                        clampf(g->gamma.y, 0.05f, 8.0f),
                        clampf(g->gamma.z, 0.05f, 8.0f));
    g->gain  = vec3_new(clampf(g->gain.x, 0.0f, 8.0f),
                        clampf(g->gain.y, 0.0f, 8.0f),
                        clampf(g->gain.z, 0.0f, 8.0f));
}

float tonemap_grade_luma(vec3 c) {
    return c.x * TONEMAP_LUMA_R + c.y * TONEMAP_LUMA_G + c.z * TONEMAP_LUMA_B;
}

vec3 tonemap_grade_white_balance(const tonemap_grade *g) {
    // crude but effective: temperature pushes the red/blue balance, tint pushes
    // green/magenta. the scale is small (a few percent per unit) so the knobs
    // feel like nudges, not nukes. centered on (1,1,1) so 0,0 is identity.
    float t = g->temperature;
    float n = g->tint;

    float r = 1.0f + 0.18f * t - 0.06f * n;
    float gg = 1.0f + 0.12f * n;
    float b = 1.0f - 0.18f * t - 0.06f * n;

    return vec3_new(clampf(r, 0.0f, 2.0f),
                    clampf(gg, 0.0f, 2.0f),
                    clampf(b, 0.0f, 2.0f));
}

// lift/gamma/gain on one channel. asc-cdl-ish: out = (in*gain + lift)^(1/gamma)
static float lgg_channel(float x, float lift, float gamma, float gain) {
    float v = x * gain + lift;
    if (v < 0.0f) v = 0.0f;
    return powf(v, 1.0f / gamma);
}

vec3 tonemap_grade_apply(const tonemap_grade *g, vec3 c) {
    // 1. white balance
    vec3 wb = tonemap_grade_white_balance(g);
    c = vec3_mul(c, wb);

    // 2. lift / gamma / gain
    c = vec3_new(lgg_channel(c.x, g->lift.x, g->gamma.x, g->gain.x),
                 lgg_channel(c.y, g->lift.y, g->gamma.y, g->gain.y),
                 lgg_channel(c.z, g->lift.z, g->gamma.z, g->gain.z));

    // 3. contrast about middle grey
    float mid = TONEMAP_MIDDLE_GREY;
    c = vec3_new((c.x - mid) * g->contrast + mid,
                 (c.y - mid) * g->contrast + mid,
                 (c.z - mid) * g->contrast + mid);

    // 4. saturation: lerp from luma-grey toward the color
    float l = tonemap_grade_luma(c);
    vec3 grey = vec3_new(l, l, l);
    c = vec3_lerp(grey, c, g->saturation);

    return clamp01(c);
}
