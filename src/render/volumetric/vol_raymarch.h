#ifndef RENDER_VOLUMETRIC_RAYMARCH_H
#define RENDER_VOLUMETRIC_RAYMARCH_H
#include "vol_phase.h"
#include "vol_dither.h"
#include "../../math/vec3.h"
#include "../../math/mat4.h"
// the cpu reference raymarcher. this is the ground truth the .frag is checked
typedef int (*volumetric_shadow_fn)(void *user, vec3 world);
typedef struct {
    volumetric_medium medium;     // validated scattering params
    vec3   to_sun;                // normalized direction toward the sun
    vec3   sun_color;             // light colour (already * intensity)

    int    steps;                 // samples along each ray
    float  max_dist;              // clamp on march length, world units

    const volumetric_dither *dither;  // start-offset jitter (may be NULL)

    volumetric_shadow_fn shadow;  // lit/occluded test
    void  *shadow_user;
} volumetric_march_ctx;
vec3 volumetric_march_ray(const volumetric_march_ctx *ctx,
                          vec3 origin, vec3 target,
                          int px, int py,
                          float *transmittance_out);
vec3 volumetric_world_from_depth(mat4 inv_view_proj,
                                 float ndc_x, float ndc_y, float depth01);
#endif
