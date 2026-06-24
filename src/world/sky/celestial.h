#ifndef WORLD_SKY_CELESTIAL_H
#define WORLD_SKY_CELESTIAL_H

// where the sun and moon sit in the sky for a given hour. this is a toy
// model: the sun rides a circle that crosses the horizon at 6 and 18, and
// the moon is just the opposite point on the circle. unit-length dirs.

#include "../../math/vec3.h"

typedef struct {
    vec3  dir;        // unit direction from the world toward the body
    float altitude;   // -1..1, sin of angle above horizon (1 = straight up)
    int   visible;    // 1 if above the horizon
} voxl_sky_body;

// sun position for the hour.
voxl_sky_body voxl_sky_sun(float hour);

// moon position for the hour (roughly opposite the sun).
voxl_sky_body voxl_sky_moon(float hour);

// convenience: 0..1 how high the sun is (clamped altitude). 0 at/below horizon.
float voxl_sky_sun_altitude01(float hour);

#endif
