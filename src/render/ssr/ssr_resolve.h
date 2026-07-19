#ifndef RENDER_SSR_RESOLVE_H
#define RENDER_SSR_RESOLVE_H
#include "ssr_ray.h"
#include "ssr_march.h"
#include "ssr_material.h"
#include "../../math/vec3.h"
#include "../../math/vec4.h"
// the resolve stage: turn a march result into an actual reflection contribution
// for one fragment. this is the cpu twin of ssrx_resolve.frag.
//
// inputs: the march outcome, the ray (for fresnel angle + backface), the
// surface material, and a way to sample the lit color buffer at the hit uv.
// output: rgb reflection color premultiplied by its confidence weight, plus the
// weight itself in .w so the caller can blend:
// final = mix(scene, reflect.rgb/reflect.w, reflect.w)  (guard w==0)
// signature for "sample the lit scene color at this uv". the pass plugs in a
// cpu buffer reader; tests plug in a flat color.
typedef vec3 (*ssrx_color_fn)(void *user, vec2 uv);
typedef struct {
    float strength;      // master blend knob (config default)
    float edge_fade;     // border width fraction
    float dist_fade;     // far-end taper fraction
    float rough_cutoff;  // roughness past which ssr is dropped
    float base_f0;       // dielectric fresnel baseline
} ssrx_resolve_params;
void ssrx_resolve_params_default(ssrx_resolve_params *p);
// resolve one fragment. returns rgb*weight in xyz, weight in w. on a miss (or a
// culled ray, or a too-rough surface) returns all zeros — no contribution.
vec4 ssrx_resolve_fragment(const ssrx_march_result *hit,
                           const ssrx_ray *ray,
                           const ssrx_material *mat,
                           const ssrx_resolve_params *p,
                           ssrx_color_fn sample_color, void *user);
#endif
