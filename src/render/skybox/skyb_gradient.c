#include "skyb_gradient.h"
#include "skyb_dither.h"
#include <math.h>
#include <stdlib.h>
skyb_rgb skyb_gradient_eval(const skyb_gradient *g, vec3 dir) {
    dir = vec3_normalize(dir);

    // vertical blend by altitude. dir.y in [-1,1]; horizon at 0. we push the
    // horizon band a little wider than a straight lerp so it reads as a band,
    // not a hard line. pow curve does that.
    float t = skyb_sat(dir.y);
    float band = powf(t, 0.45f);              // bias color toward horizon
    skyb_rgb col = skyb_mix(g->horizon, g->zenith, band);

    // haze: lift the whole lower sky toward the horizon color near the ground
    float low = skyb_smooth(0.35f, 0.0f, dir.y);
    col = skyb_mix(col, g->horizon, low * g->haze);

    // sun scatter glow: a soft halo around the sun direction. dot falloff,
    // tinted by sun_tint, gated by how much light the sun is giving.
    float d = vec3_dot(dir, g->sun_dir);
    if (d > 0.0f && g->sun_light > 0.0f) {
        // tight core + wide skirt, both clamped
        float core = powf(skyb_sat(d), 64.0f);
        float skirt = powf(skyb_sat(d), 4.0f) * 0.35f;
        float glow = (core + skirt) * g->sun_light;
        col = skyb_mix(col, g->sun_tint, skyb_sat(glow));
    }

    // a faint warm scatter opposite the sun low on the horizon (belt of venus
    // -ish). cheap: only when the sun is low.
    float horizon_amt = skyb_smooth(0.25f, 0.0f, dir.y);
    float anti = skyb_sat(-d) * horizon_amt * g->sun_light * 0.15f;
    if (anti > 0.0f) col = skyb_mix(col, g->sun_tint, anti);

    // optional analytic scatter on top of the hand-rolled glow. additive.
    if (g->use_scatter && g->sun_light > 0.0f) {
        skyb_rgb add = skyb_scatter_eval(&g->scatter, dir, g->sun_dir,
                                         g->sun_tint, g->sun_light);
        col.x = skyb_sat(col.x + add.x);
        col.y = skyb_sat(col.y + add.y);
        col.z = skyb_sat(col.z + add.z);
    }

    return col;
}

// push one vertex, growing the buffer geometrically.
static void push(skyb_dome *d, vec3 dir, skyb_rgb c) {
    if (d->count == d->cap) {
        int nc = d->cap ? d->cap * 2 : 256;
skyb_dome_vertex *nv = realloc(d->verts, nc * sizeof *nv);
if (!nv) return;
d->verts = nv;
d->cap = nc;
}
    skyb_dome_vertex *v = &d->verts[d->count++];
v->x = dir.x;
v->y = dir.y;
v->z = dir.z;
v->r = c.x;
v->g = c.y;
v->b = c.z;
}

// direction on the dome for ring i (0=top) / sector j. rings span from the
// zenith down to a little below the horizon so the skirt overlaps geometry.
static vec3 dome_dir(int i, int j, int rings, int sectors) {
    // phi: 0 at zenith, slightly past pi/2 at the bottom skirt
    float phi = (SKYB_PI * 0.5f + 0.12f) * ((float)i / (float)rings);
    float th  = SKYB_TAU * ((float)j / (float)sectors);
    vec3 d;
    d.x = sinf(phi) * cosf(th);
    d.y = cosf(phi);
    d.z = sinf(phi) * sinf(th);
    return d; // already unit
}

void skyb_dome_build(skyb_dome *d, int rings, int sectors, float radius) {
    d->verts = NULL;
d->count = 0;
d->cap = 0;
d->rings = rings;
d->sectors = sectors;
d->radius = radius;
;
for (int i = 0;
i < rings;
i < d->count;
d->verts = NULL;
d->count = d->cap = 0;
}
