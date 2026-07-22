#ifndef RENDER_VOLUMETRIC_RAYMARCH_H
#define RENDER_VOLUMETRIC_RAYMARCH_H

#include "vol_phase.h"
#include "vol_dither.h"
#include "../../math/vec3.h"
#include "../../math/mat4.h"

// the cpu reference raymarcher. this is the ground truth the .frag is checked
// against — it runs headless in tests and, in a pinch, can fill the scatter
// buffer on the cpu when the gpu path is disabled (slow, but correct).
//
// for each pixel we walk a ray from the camera toward the world position the
// depth buffer says is there, sampling the sun's shadow map at every step. a
// step that's lit adds in-scattered light weighted by the hg phase and the
// accumulated transmittance; a step in shadow adds nothing. dithering offsets
// the first step per pixel.

// a shadow query. given a world point, return 1 if the sun can see it (lit),
// 0 if it's occluded. the gpu does this with a shadow-map compare; on the cpu
// the tests hand in a closure over their fake world. `user` is opaque.
typedef int (*volumetric_shadow_fn)(void *user, vec3 world);

// everything the marcher needs that isn't a per-ray input. assembled once per
// frame by the pass.
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

// march a single ray. `origin` is the camera/eye, `target` is the reconstructed
// world position of the surface this pixel hit (the ray stops there). `px,py`
// pick the dither cell. returns the accumulated in-scattered radiance; on
// return *transmittance_out (if non-NULL) holds the surviving transmittance
// along the ray, which the composite uses to dim the background.
vec3 volumetric_march_ray(const volumetric_march_ctx *ctx,
                          vec3 origin, vec3 target,
                          int px, int py,
                          float *transmittance_out);

// convenience: reconstruct a world position from a depth value and the inverse
// view-projection. depth01 is the [0,1] depth-buffer value, ndc.xy in [-1,1].
vec3 volumetric_world_from_depth(mat4 inv_view_proj,
                                 float ndc_x, float ndc_y, float depth01);

#endif
