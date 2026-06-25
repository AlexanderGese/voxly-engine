#include "rain.h"
#include "sky_math.h"
#include "sky_color.h"

static void respawn(voxl_sky_rain *r, voxl_sky_drop *d, vec3 center) {
    d->pos.x = center.x + voxl_sky_rand_range(&r->rng, -r->box, r->box);
    d->pos.z = center.z + voxl_sky_rand_range(&r->rng, -r->box, r->box);
    d->pos.y = center.y + r->top * voxl_sky_rand_range(&r->rng, 0.5f, 1.0f);
    d->vy  = -voxl_sky_rand_range(&r->rng, 18.0f, 26.0f);   // rain falls fast
    d->len = voxl_sky_rand_range(&r->rng, 0.6f, 1.4f);
    d->alive = 1;
}

void voxl_sky_rain_init(voxl_sky_rain *r, unsigned seed, float box, float top) {
    voxl_sky_rand_seed(&r->rng, (uint32_t)seed);
    r->count = 0;
    r->box = box > 1.0f ? box : 1.0f;
    r->top = top > 1.0f ? top : 1.0f;
    r->wind = (vec3){1.5f, 0.0f, 0.0f};
    for (int i = 0; i < VOXL_SKY_RAIN_MAX; i++) {
        r->drops[i].alive = 0;
    }
}

void voxl_sky_rain_set_intensity(voxl_sky_rain *r, float intensity) {
    intensity = voxl_sky_clampf(intensity, 0.0f, 1.0f);
    r->count = (int)(intensity * VOXL_SKY_RAIN_MAX);
    if (r->count > VOXL_SKY_RAIN_MAX) r->count = VOXL_SKY_RAIN_MAX;
}

void voxl_sky_rain_update(voxl_sky_rain *r, float dt, vec3 center) {
    if (dt < 0.0f) dt = 0.0f;
    for (int i = 0; i < r->count; i++) {
        voxl_sky_drop *d = &r->drops[i];
        if (!d->alive) {
            respawn(r, d, center);
            continue;
        }
        d->pos.y += d->vy * dt;
        d->pos.x += r->wind.x * dt;
        d->pos.z += r->wind.z * dt;
        // hit the ground plane or wandered too low -> recycle.
        if (d->pos.y <= center.y - 1.0f) {
            respawn(r, d, center);
        }
    }
    // any drops beyond the active count get parked.
    for (int i = r->count; i < VOXL_SKY_RAIN_MAX; i++) {
        r->drops[i].alive = 0;
    }
}

vec4 voxl_sky_rain_color(float hour, float intensity) {
    intensity = voxl_sky_clampf(intensity, 0.0f, 1.0f);
    // base rain is a desaturated blue-grey; pick brightness from daylight.
    float day = voxl_sky_sun_brightness(hour);
    float b = voxl_sky_lerpf(0.35f, 0.70f, day);
    vec4 c;
    c.x = b * 0.62f;
    c.y = b * 0.68f;
    c.z = b * 0.80f;
    c.w = voxl_sky_lerpf(0.15f, 0.45f, intensity);
    return c;
}
