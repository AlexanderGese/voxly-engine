#ifndef WORLD_SKY_STARFIELD_H
#define WORLD_SKY_STARFIELD_H

// a fixed set of stars scattered on the sky dome. positions are baked once
// from a seed; brightness fades in at night and twinkles a little over time.

#include "../../math/vec3.h"

#define VOXL_SKY_STAR_MAX 512

typedef struct {
    vec3  dir;        // unit direction on the sky dome
    float base;       // baseline brightness 0..1
    float twinkle;    // per-star twinkle speed
} voxl_sky_star;

typedef struct {
    voxl_sky_star stars[VOXL_SKY_STAR_MAX];
    int           count;
} voxl_sky_starfield;

// bake `count` stars (clamped to VOXL_SKY_STAR_MAX) from a seed.
void voxl_sky_starfield_init(voxl_sky_starfield *sf, int count, unsigned seed);

// overall star visibility for the hour: 0 in day, 1 deep night.
float voxl_sky_star_visibility(float hour);

// brightness of one star right now (visibility * twinkle * base). 0..1.
float voxl_sky_star_brightness(const voxl_sky_star *s, float hour, float time_s);

#endif
