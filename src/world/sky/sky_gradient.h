#ifndef WORLD_SKY_GRADIENT_H
#define WORLD_SKY_GRADIENT_H

// vertical sky gradient. given the time of day (0..24) we pick a zenith
// (top of sky) and horizon color, then blend by height t (0=horizon, 1=zenith).
// returns plain rgb in vec3, 0..1 per channel. no gl here, just data.

#include "../../math/vec3.h"

typedef struct {
    vec3 zenith;    // straight up
    vec3 horizon;   // out at the edge
} voxl_sky_band;

// pick the zenith+horizon pair for a given hour.
voxl_sky_band voxl_sky_gradient_band(float hour);

// full gradient sample: hour + vertical t in [0,1].
vec3 voxl_sky_gradient_at(float hour, float t);

#endif
