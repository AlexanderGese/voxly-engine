#ifndef WORLD_SKY_AMBIENT_LIGHT_H
#define WORLD_SKY_AMBIENT_LIGHT_H

// ambient/sky light terms for shading the world. returns a directional sun
// color + an ambient fill that the renderer can multiply into block light.
// weather (wetness 0..1) dims everything a bit.

#include "../../math/vec3.h"

typedef struct {
    vec3  sun_dir;     // unit direction toward the sun
    vec3  sun_color;   // directional light color (already scaled by strength)
    vec3  ambient;     // flat fill light
    float strength;    // 0..1 overall daylight strength
} voxl_sky_lighting;

// compute the full lighting set for an hour and weather wetness.
voxl_sky_lighting voxl_sky_lighting_at(float hour, float wetness);

// just the 0..1 ambient scalar (handy for simple shaders).
float voxl_sky_ambient_level(float hour, float wetness);

#endif
