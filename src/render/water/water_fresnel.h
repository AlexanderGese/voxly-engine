#ifndef RENDER_WATER_WATER_FRESNEL_H
#define RENDER_WATER_WATER_FRESNEL_H

#include "../../math/vec3.h"

// the optics bits. fresnel reflectance, the reflect/refract vectors, and the
// depth-based tint blend. all cpu-side helpers — the real per-pixel work lives
// in the shader but i mirror it here for previews / picking / sanity checks.

// schlick fresnel for a view direction against a surface normal. both must be
// unit. returns a reflectance in [WATER_FRESNEL_MIN, WATER_FRESNEL_MAX].
float water_fresnel_schlick(vec3 view_dir, vec3 normal);

// same but you pass cos(theta) directly if you already have it.
float water_fresnel_from_cos(float cos_theta);

// classic reflect: i - 2*(i.n)*n. i points toward the surface.
vec3  water_reflect(vec3 incident, vec3 normal);

// snells-law refraction air->water (eta = 1/1.33). returns 0 and leaves *out
// untouched on total internal reflection (cant happen entering denser medium,
// but the guard is there for the underwater case).
int   water_refract(vec3 incident, vec3 normal, float eta, vec3 *out);

// blend shallow tint -> deep tint by how many blocks of water the view ray
// travels before hitting the bottom. depth clamped at WATER_TINT_DEPTH.
vec3  water_depth_tint(float depth_blocks);

// combine reflection and refraction colors by a fresnel factor. straight lerp,
// but kept here so the cpu preview matches the shader exactly.
vec3  water_mix_fresnel(vec3 refraction_rgb, vec3 reflection_rgb, float fresnel);

#endif
