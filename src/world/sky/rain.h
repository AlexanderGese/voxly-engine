#ifndef WORLD_SKY_RAIN_H
#define WORLD_SKY_RAIN_H

// cpu rain field. a fixed pool of drops falling in a box around the player.
// when a drop hits the bottom (or dies) it respawns at the top. this just
// owns positions/velocities/colors as data; no rendering here.

#include "../../math/vec3.h"
#include "../../math/vec4.h"
#include "sky_rand.h"

#define VOXL_SKY_RAIN_MAX 2048

typedef struct {
    vec3  pos;     // world position
    float vy;      // fall speed (negative = down)
    float len;     // streak length (for drawing a line)
    int   alive;
} voxl_sky_drop;

typedef struct {
    voxl_sky_drop drops[VOXL_SKY_RAIN_MAX];
    int    count;        // active drops
    float  box;          // half-extent of the spawn box around center
    float  top;          // spawn height above center
    vec3   wind;         // horizontal drift
    voxl_sky_rand rng;
} voxl_sky_rain;

void voxl_sky_rain_init(voxl_sky_rain *r, unsigned seed, float box, float top);

// set how many drops should be live based on intensity 0..1.
void voxl_sky_rain_set_intensity(voxl_sky_rain *r, float intensity);

// move drops; recycle ones that fall past `center.y`.
void voxl_sky_rain_update(voxl_sky_rain *r, float dt, vec3 center);

// rgba color for rain, slightly time-of-day tinted. alpha grows with intensity.
vec4 voxl_sky_rain_color(float hour, float intensity);

#endif
