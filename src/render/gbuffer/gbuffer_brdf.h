#ifndef RENDER_GBUFFER_BRDF_H
#define RENDER_GBUFFER_BRDF_H

#include "../../math/vec3.h"
#include "gbuffer_material.h"
#include "gbuffer_light.h"

// cpu-side reference brdf. the real shading happens in the accum fragment
// shader, but we keep a 1:1 c implementation here for three reasons:
// - the light cull uses the same attenuation curve, so it lives in one
// place and can't drift from the shader
// - entity/particle code that isn't deferred can call this directly to
// match the look of the deferred world
// - it's trivially unit-testable, unlike glsl
//
// cook-torrance-ish: ggx distribution, schlick fresnel, smith-ggx geometry.
// nothing exotic. all colors are linear rgb.

// distance attenuation. matches gbuffer_light's inverse-square + radius
// window exactly. d is the light-to-surface distance.
float gbuffer_brdf_attenuation(float d, float radius);

// schlick fresnel. f0 is the surface reflectance at normal incidence.
vec3 gbuffer_brdf_fresnel(float cos_theta, vec3 f0);

// ggx normal distribution term. roughness is perceptual (0..1).
float gbuffer_brdf_distribution_ggx(float n_dot_h, float roughness);

// smith geometry term using schlick-ggx, combined for view + light.
float gbuffer_brdf_geometry_smith(float n_dot_v, float n_dot_l, float roughness);

// full single-light evaluation. returns the outgoing radiance contribution
// for one light at this surface point. n/v/l are unit vectors; l points from
// the surface toward the light.
vec3 gbuffer_brdf_eval(vec3 n, vec3 v, vec3 l, vec3 light_color,
                       vec3 albedo, gbuffer_material mat);

// convenience: evaluate a whole gbuffer_light against a surface point. does
// the direction + attenuation setup then defers to gbuffer_brdf_eval.
vec3 gbuffer_brdf_eval_light(const gbuffer_light *li, vec3 world_pos,
                             vec3 n, vec3 v, vec3 albedo, gbuffer_material mat);

#endif
