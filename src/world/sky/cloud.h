#ifndef WORLD_SKY_CLOUD_H
#define WORLD_SKY_CLOUD_H

// a coarse 2d cloud layer. value-noise on a grid that scrolls with the wind.
// sample coverage at a world (x,z) to get 0..1 cloud density. weather pushes
// coverage up. self-contained value noise, no engine noise dependency.

#include "../../math/vec3.h"

typedef struct {
    float scroll_x;    // accumulated wind offset
    float scroll_z;
    float wind_x;      // blocks/sec
    float wind_z;
    float scale;       // world units per noise cell
    float coverage;    // 0..1 bias added to the noise threshold
    unsigned seed;
} voxl_sky_clouds;

void voxl_sky_clouds_init(voxl_sky_clouds *c, unsigned seed);

// advance the scroll. weather wetness raises coverage.
void voxl_sky_clouds_update(voxl_sky_clouds *c, float dt, float wetness);

// 0..1 cloud density at a world xz position.
float voxl_sky_clouds_density(const voxl_sky_clouds *c, float wx, float wz);

// shaded cloud color for the hour (lit top vs shadowed underside mix at t).
vec3 voxl_sky_cloud_color(float hour, float t);

#endif
