#include "skyb_palette.h"

#include <math.h>

static skyb_rgb rgb(float r, float g, float b) {
    skyb_rgb c = { r, g, b };
    return c;
}

void skyb_palette_default(skyb_palette *p) {
    p->count = 0;
    // night -> astronomical dawn -> sunrise -> day -> sunset -> dusk -> night.
    // values eyeballed, not physically derived. looked fine, shipped it.
    skyb_key def[] = {
        { 0.0f,  rgb(0.01f, 0.01f, 0.04f), rgb(0.02f, 0.02f, 0.06f), rgb(0.20f, 0.22f, 0.40f) },
        { 5.0f,  rgb(0.04f, 0.05f, 0.12f), rgb(0.10f, 0.08f, 0.16f), rgb(0.45f, 0.35f, 0.45f) },
        { 6.5f,  rgb(0.20f, 0.32f, 0.55f), rgb(0.85f, 0.45f, 0.30f), rgb(1.00f, 0.55f, 0.30f) },
        { 8.0f,  rgb(0.30f, 0.52f, 0.82f), rgb(0.70f, 0.80f, 0.92f), rgb(1.00f, 0.92f, 0.80f) },
        { 12.0f, rgb(0.32f, 0.58f, 0.92f), rgb(0.66f, 0.82f, 0.96f), rgb(1.00f, 1.00f, 0.98f) },
        { 16.0f, rgb(0.30f, 0.54f, 0.86f), rgb(0.72f, 0.78f, 0.90f), rgb(1.00f, 0.96f, 0.86f) },
        { 17.5f, rgb(0.24f, 0.36f, 0.62f), rgb(0.92f, 0.52f, 0.30f), rgb(1.00f, 0.58f, 0.28f) },
        { 19.0f, rgb(0.08f, 0.10f, 0.24f), rgb(0.40f, 0.20f, 0.24f), rgb(0.70f, 0.35f, 0.34f) },
        { 21.0f, rgb(0.02f, 0.02f, 0.07f), rgb(0.05f, 0.04f, 0.10f), rgb(0.30f, 0.28f, 0.44f) },
    };
    int n = (int)(sizeof def / sizeof def[0]);
    for (int i = 0; i < n; i++) skyb_palette_add(p, def[i]);
}

void skyb_palette_add(skyb_palette *p, skyb_key k) {
    if (p->count >= SKYB_PALETTE_MAX) return;
    k.hour = skyb_wrap24(k.hour);
    // insertion sort by hour, keeps sampling simple
    int i = p->count - 1;
    while (i >= 0 && p->keys[i].hour > k.hour) {
        p->keys[i + 1] = p->keys[i];
        i--;
    }
    p->keys[i + 1] = k;
    p->count++;
}

// find the two keys bracketing `hour` and the 0..1 fraction between them.
// wraps across midnight: the last key blends into the first +24h.
static void bracket(const skyb_palette *p, float hour,
                    const skyb_key **a, const skyb_key **b, float *frac) {
    int n = p->count;
    *a = &p->keys[n - 1];
    *b = &p->keys[0];

    float ha = p->keys[n - 1].hour - 24.0f; // previous-day instance
    float hb = p->keys[0].hour;

    for (int i = 0; i < n; i++) {
        float h0 = p->keys[i].hour;
        float h1 = (i + 1 < n) ? p->keys[i + 1].hour : p->keys[0].hour + 24.0f;
        if (hour >= h0 && hour < h1) {
            *a = &p->keys[i];
            *b = &p->keys[(i + 1) % n];
            ha = h0;
            hb = h1;
            break;
        }
    }

    float span = hb - ha;
    *frac = (span > 1e-4f) ? (hour - ha) / span : 0.0f;
}

skyb_sky_colors skyb_palette_sample(const skyb_palette *p, float hour) {
    skyb_sky_colors out;
    if (p->count == 0) {
        out.zenith = out.horizon = out.sun_tint = (skyb_rgb){0, 0, 0};
        return out;
    }
    if (p->count == 1) {
        out.zenith   = p->keys[0].zenith;
        out.horizon  = p->keys[0].horizon;
        out.sun_tint = p->keys[0].sun_tint;
        return out;
    }

    hour = skyb_wrap24(hour);
    const skyb_key *a, *b;
    float f;
    bracket(p, hour, &a, &b, &f);

    // smoothstep the fraction so we ease in/out of each key
    float t = f * f * (3.0f - 2.0f * f);
    out.zenith   = skyb_mix(a->zenith,   b->zenith,   t);
    out.horizon  = skyb_mix(a->horizon,  b->horizon,  t);
    out.sun_tint = skyb_mix(a->sun_tint, b->sun_tint, t);
    return out;
}
