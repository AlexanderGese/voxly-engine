#ifndef WORLD_SKY_SNOW_H
#define WORLD_SKY_SNOW_H

// snow field. like rain but slow, with a sideways sway so flakes drift.
// shares the same spawn-box / recycle idea. data only, no rendering.

#include "../../math/vec3.h"
#include "../../math/vec4.h"
#include "sky_rand.h"

#define VOXL_SKY_SNOW_MAX 1536

typedef struct {
    vec3  pos;
    float vy;       // gentle fall
    float sway;     // horizontal sway amplitude
    float phase;    // sway phase offset
    float size;     // flake size
    int   alive;
} voxl_sky_flake;

typedef struct {
    voxl_sky_flake flakes[VOXL_SKY_SNOW_MAX];
    int   count;
    float box;
    float top;
    float t;        // accumulated time, drives the sway
    voxl_sky_rand rng;
} voxl_sky_snow;

void voxl_sky_snow_init(voxl_sky_snow *s, unsigned seed, float box, float top);
void voxl_sky_snow_set_intensity(voxl_sky_snow *s, float intensity);
void voxl_sky_snow_update(voxl_sky_snow *s, float dt, vec3 center);

// snow color: white, faintly tinted by sky, alpha from intensity.
vec4 voxl_sky_snow_color(float hour, float intensity);

#endif
