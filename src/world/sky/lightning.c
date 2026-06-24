#include "lightning.h"
#include "sky_math.h"

#include <math.h>

// how fast the flash fades, per second.
#define VOXL_SKY_FLASH_DECAY 4.0f

void voxl_sky_lightning_init(voxl_sky_lightning *l, unsigned seed) {
    voxl_sky_rand_seed(&l->rng, (uint32_t)seed);
    l->flash = 0.0f;
    l->cooldown = 2.0f;
    l->last_strike = (vec3){0.0f, 0.0f, 0.0f};
    l->struck = 0;
}

int voxl_sky_lightning_update(voxl_sky_lightning *l, float dt,
                              float storm_intensity, vec3 center) {
    if (dt < 0.0f) dt = 0.0f;
    l->struck = 0;

    // fade any active flash.
    if (l->flash > 0.0f) {
        l->flash = voxl_sky_clampf(l->flash - VOXL_SKY_FLASH_DECAY * dt, 0.0f, 1.0f);
    }

    storm_intensity = voxl_sky_clampf(storm_intensity, 0.0f, 1.0f);
    if (storm_intensity <= 0.05f) {
        l->cooldown = 2.0f;   // no storm, reset the clock
        return 0;
    }

    l->cooldown -= dt;
    if (l->cooldown > 0.0f) return 0;

    // time to roll. stronger storms strike sooner on the next cycle.
    float lo = voxl_sky_lerpf(8.0f, 1.5f, storm_intensity);
    float hi = voxl_sky_lerpf(20.0f, 5.0f, storm_intensity);
    l->cooldown = voxl_sky_rand_range(&l->rng, lo, hi);

    // bolt brightness scales with intensity, with a little randomness.
    l->flash = voxl_sky_clampf(voxl_sky_rand_range(&l->rng, 0.6f, 1.0f)
                               * (0.5f + 0.5f * storm_intensity), 0.0f, 1.0f);

    // scatter the strike around the center on the xz plane.
    float r  = voxl_sky_rand_range(&l->rng, 8.0f, 64.0f);
    float az = voxl_sky_rand_range(&l->rng, 0.0f, VOXL_SKY_TAU);
    l->last_strike.x = center.x + r * cosf(az);
    l->last_strike.y = 0.0f;
    l->last_strike.z = center.z + r * sinf(az);

    l->struck = 1;
    return 1;
}

float voxl_sky_lightning_flash(const voxl_sky_lightning *l) {
    return l->flash;
}
