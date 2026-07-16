#ifndef RENDER_SKYBOX_COMMON_H
#define RENDER_SKYBOX_COMMON_H

// shared scratch for the render/skybox module. small pure helpers + a couple
// of common little structs. kept separate from world/sky so the renderer side
// can iterate on look without touching the simulation side.
//
// prefix for everything public here is skyb_. don't collide with the sky_*
// sim module or the old flat render/skybox.c.

#include "../../math/vec3.h"
#include "../../math/vec4.h"

#define SKYB_PI   3.14159265358979f
#define SKYB_TAU  6.28318530717958f
#define SKYB_DEG  (SKYB_PI / 180.0f)

// an rgb color is just a vec3 here. alias for intent, not a new type.
typedef vec3 skyb_rgb;

// clamp into [lo,hi].
float skyb_clampf(float v, float lo, float hi);

// clamp into [0,1]. used everywhere so it gets its own name.
float skyb_sat(float v);

// plain lerp, t not clamped.
float skyb_lerpf(float a, float b, float t);

// glsl-style smoothstep, returns 0..1, edges may be in either order.
float skyb_smooth(float e0, float e1, float x);

// remap x from [a,b] -> [c,d], safe when a==b.
float skyb_remap(float x, float a, float b, float c, float d);

// component lerp of colors.
skyb_rgb skyb_mix(skyb_rgb a, skyb_rgb b, float t);

// scale a color, clamped back into [0,1] per channel.
skyb_rgb skyb_rgb_scale(skyb_rgb c, float s);

// cheap srgb-ish gamma. we keep the sky in linear-ish space and only bend it
// at the very end so the horizon doesn't blow out. exponent 1/2.2 hardcoded.
skyb_rgb skyb_tonemap(skyb_rgb c, float exposure);

// wrap hour into [0,24).
float skyb_wrap24(float h);

// fract part in [0,1).
float skyb_fract(float x);

// build a unit dir from altitude (radians above horizon) and azimuth.
vec3 skyb_dir_from_angles(float altitude, float azimuth);

#endif
