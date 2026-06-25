#ifndef WORLD_SKY_COLOR_H
#define WORLD_SKY_COLOR_H

// higher level sky colors derived from the time of day: the fog/clear color,
// the sunlight tint, and the ambient term. these feed the renderer as data.

#include "../../math/vec3.h"

// the flat fog/background color (roughly the horizon color at the given hour).
vec3 voxl_sky_fog_color(float hour);

// sunlight color. warm at dawn/dusk, white-ish at noon, cool blue at night.
vec3 voxl_sky_sun_tint(float hour);

// 0..1 brightness of the sun disc over the day.
float voxl_sky_sun_brightness(float hour);

// tint sky colors by a wetness factor (0 clear .. 1 storm): desaturates and
// darkens toward a grey overcast. handy when weather is active.
vec3 voxl_sky_apply_overcast(vec3 color, float wetness);

#endif
