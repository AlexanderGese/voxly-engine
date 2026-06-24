#ifndef WORLD_SKY_LIGHTNING_H
#define WORLD_SKY_LIGHTNING_H

// storm lightning: rolls random strikes when intensity is high enough, and
// tracks a short sky-flash that decays over a fraction of a second. produces
// a flash brightness 0..1 the renderer can add to the sky/ambient.

#include "../../math/vec3.h"
#include "sky_rand.h"

typedef struct {
    float flash;        // current flash brightness 0..1
    float cooldown;     // seconds until the next eligible strike roll
    vec3  last_strike;  // where the last bolt hit (world-ish xz, y=0)
    int   struck;       // set to 1 on the frame a bolt fires, else 0
    voxl_sky_rand rng;
} voxl_sky_lightning;

void voxl_sky_lightning_init(voxl_sky_lightning *l, unsigned seed);

// step it. storm_intensity 0..1 drives strike frequency. center is the
// player/world position bolts scatter around. returns 1 if a bolt fired.
int voxl_sky_lightning_update(voxl_sky_lightning *l, float dt,
                              float storm_intensity, vec3 center);

// current flash brightness (also readable directly from the struct).
float voxl_sky_lightning_flash(const voxl_sky_lightning *l);

#endif
