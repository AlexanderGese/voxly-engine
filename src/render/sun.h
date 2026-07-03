#ifndef RENDER_SUN_H
#define RENDER_SUN_H

#include "../math/vec3.h"

// directional sun position from a day hour. used for dot-product shading
// and to place a sun quad in the sky.

vec3 sun_direction(float hours);
void sun_color(float hours, float *r, float *g, float *b);

#endif
