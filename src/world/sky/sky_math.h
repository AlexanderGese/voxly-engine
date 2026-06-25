#ifndef WORLD_SKY_MATH_H
#define WORLD_SKY_MATH_H

// tiny shared math helpers for the sky/weather modules.
// kept self-contained so the sky stuff doesn't drag in the whole engine.
// all funcs are pure (no globals) and operate on plain floats / vec3 / vec4.

#include "../../math/vec3.h"
#include "../../math/vec4.h"

#define VOXL_SKY_PI 3.14159265358979f
#define VOXL_SKY_TAU 6.28318530717958f

// clamp a float into [lo,hi].
float voxl_sky_clampf(float v, float lo, float hi);

// linear blend between a and b. t is NOT clamped (callers usually pre-clamp).
float voxl_sky_lerpf(float a, float b, float t);

// remap x from [a,b] onto [c,d]. safe when a==b (returns c).
float voxl_sky_remap(float x, float a, float b, float c, float d);

// hermite smoothstep, like glsl. returns 0..1.
float voxl_sky_smooth(float edge0, float edge1, float x);

// component-wise lerp of two colors.
vec3 voxl_sky_mix3(vec3 a, vec3 b, float t);
vec4 voxl_sky_mix4(vec4 a, vec4 b, float t);

// wrap an hour value into [0,24).
float voxl_sky_wrap24(float h);

// fract part of x in [0,1).
float voxl_sky_fract(float x);

#endif
