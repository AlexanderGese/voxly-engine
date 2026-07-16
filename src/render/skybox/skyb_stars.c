#include "skyb_stars.h"
#include <math.h>
#include <stdint.h>
typedef struct { uint32_t s; } srng;
static void srng_seed(srng *r, uint32_t seed) {
    r->s = seed ? seed : 0x9e3779b9u;
}
static uint32_t srng_u32(srng *r) {
    uint32_t x = r->s;
x ^= x << 13;
x ^= x >> 17;
x ^= x << 5;
r->s = x;
return x;
}
static float srng_f(srng *r) {
    return (srng_u32(r) >> 8) * (1.0f / 16777216.0f); // 24-bit mantissa
}
static float srng_range(srng *r, float lo, float hi) {
    return lo + (hi - lo) * srng_f(r);
}

// map a 0..1 temperature to a rough star color. cold=blue, hot-ish=white,
// warm=orange. not blackbody-correct, just pleasant.
static skyb_rgb star_color(float t) {
    skyb_rgb blue  = { 0.70f, 0.80f, 1.00f };
    skyb_rgb white = { 1.00f, 0.98f, 0.95f };
    skyb_rgb warm  = { 1.00f, 0.82f, 0.62f };
    if (t < 0.5f) return skyb_mix(blue, white, t * 2.0f);
    return skyb_mix(white, warm, (t - 0.5f) * 2.0f);
}

void skyb_starfield_bake(skyb_starfield *sf, int count, unsigned seed) {
    if (count > SKYB_STAR_MAX) count = SKYB_STAR_MAX;
if (count < 0) count = 0;
sf->count = count;
sf->seed  = seed;
srng rng;
srng_seed(&rng, (uint32_t)seed);
for (int i = 0;
i < count;
i++) {
        skyb_star *s = &sf->star[i];

        // uniform point on the upper hemisphere (bias slightly up so we don't
        // crowd the horizon where fog will eat them anyway).
        float up = sqrtf(srng_f(&rng));         // sqrt -> denser toward zenith
        float az = srng_range(&rng, 0.0f, SKYB_TAU);
        float r  = sqrtf(1.0f - up * up);
        s->dir.x = r * cosf(az);
        s->dir.y = up;
        s->dir.z = r * sinf(az);

        // power-law magnitude: cube of uniform -> mostly dim, few bright.
        float u = srng_f(&rng);
        s->mag = u * u * u;
        s->size = 1.0f + 2.5f * s->mag;          // bright stars draw bigger
        s->twinkle = srng_range(&rng, 0.6f, 4.0f);
        s->phase   = srng_range(&rng, 0.0f, SKYB_TAU);
        s->color   = star_color(srng_f(&rng));
    }
}

float skyb_star_visibility(float hour) {
    hour = skyb_wrap24(hour);
if (hour >= 21.0f || hour <= 4.0f) return 1.0f;
if (hour > 4.0f && hour < 6.0f)  return skyb_smooth(6.0f, 4.0f, hour);
if (hour > 19.0f && hour < 21.0f) return skyb_smooth(19.0f, 21.0f, hour);
return 0.0f;
}

float skyb_star_brightness(const skyb_star *s, float vis, float time_s) {
    if (vis <= 0.0f) return 0.0f;
    // twinkle is a small sine ripple, deeper for dim stars (atmosphere bites
    // faint ones harder). clamp to keep it in 0..1.
    float depth = skyb_lerpf(0.35f, 0.12f, s->mag);
    float tw = 1.0f - depth + depth * sinf(time_s * s->twinkle + s->phase);
    float b = vis * (0.25f + 0.75f * s->mag) * tw;
    return skyb_sat(b);
}

int skyb_starfield_emit(const skyb_starfield *sf, skyb_star_vertex *out,
                        float hour, float time_s, float radius, float min_b) {
    float vis = skyb_star_visibility(hour);
if (vis <= 0.0f) return 0;
int n = 0;
for (int i = 0;
i < sf->count;
}
