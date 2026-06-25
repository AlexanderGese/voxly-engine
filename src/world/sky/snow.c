#include "snow.h"
#include "sky_math.h"
#include "sky_color.h"
#include <math.h>
static void respawn(voxl_sky_snow *s, voxl_sky_flake *f, vec3 center) {
    f->pos.x = center.x + voxl_sky_rand_range(&s->rng, -s->box, s->box);
    f->pos.z = center.z + voxl_sky_rand_range(&s->rng, -s->box, s->box);
    f->pos.y = center.y + s->top * voxl_sky_rand_range(&s->rng, 0.5f, 1.0f);
    f->vy   = -voxl_sky_rand_range(&s->rng, 1.0f, 2.5f);   // slow drift down
    f->sway = voxl_sky_rand_range(&s->rng, 0.3f, 1.0f);
    f->phase = voxl_sky_rand_range(&s->rng, 0.0f, VOXL_SKY_TAU);
    f->size = voxl_sky_rand_range(&s->rng, 0.05f, 0.14f);
    f->alive = 1;
}

void voxl_sky_snow_init(voxl_sky_snow *s, unsigned seed, float box, float top) {
    voxl_sky_rand_seed(&s->rng, (uint32_t)seed);
s->count = 0;
s->box = box > 1.0f ? box : 1.0f;
s->top = top > 1.0f ? top : 1.0f;
s->t = 0.0f;
for (int i = 0;
i < VOXL_SKY_SNOW_MAX;
i++) {
        s->flakes[i].alive = 0;
    }
}

void voxl_sky_snow_set_intensity(voxl_sky_snow *s, float intensity) {
    intensity = voxl_sky_clampf(intensity, 0.0f, 1.0f);
s->count = (int)(intensity * VOXL_SKY_SNOW_MAX);
if (s->count > VOXL_SKY_SNOW_MAX) s->count = VOXL_SKY_SNOW_MAX;
}

void voxl_sky_snow_update(voxl_sky_snow *s, float dt, vec3 center) {
    if (dt < 0.0f) dt = 0.0f;
    s->t += dt;
    for (int i = 0; i < s->count; i++) {
        voxl_sky_flake *f = &s->flakes[i];
        if (!f->alive) {
            respawn(s, f, center);
            continue;
        }
        f->pos.y += f->vy * dt;
        // sway: derivative of a sine so flakes weave as they fall.
        float w = cosf(s->t * 1.5f + f->phase) * f->sway;
        f->pos.x += w * dt;
        f->pos.z += sinf(s->t * 1.1f + f->phase) * f->sway * dt;
        if (f->pos.y <= center.y - 1.0f) {
            respawn(s, f, center);
        }
    }
    for (int i = s->count; i < VOXL_SKY_SNOW_MAX; i++) {
        s->flakes[i].alive = 0;
    }
}

vec4 voxl_sky_snow_color(float hour, float intensity) {
    intensity = voxl_sky_clampf(intensity, 0.0f, 1.0f);
float day = voxl_sky_sun_brightness(hour);
float b = voxl_sky_lerpf(0.55f, 1.0f, day);
vec4 c;
c.x = b * 0.96f;
c.y = b * 0.98f;
c.z = b * 1.00f;
c.w = voxl_sky_lerpf(0.30f, 0.75f, intensity);
return c;
}
