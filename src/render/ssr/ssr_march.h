#ifndef RENDER_SSR_MARCH_H
#define RENDER_SSR_MARCH_H

#include "ssr_ray.h"
#include "ssr_gbuffer.h"
#include "ssr_depth.h"
#include "../../math/vec2.h"

// the view-space ray march. this is the cpu reference for the ssrx_reflect.frag
// linear march + binary refine. it's a tiny state machine:
//
// SEEK   walk the ray in growing strides, projecting each point to a uv and
// asking the depth buffer for the scene z there. while the marched
// point is still in *front* of the scene surface (closer to the eye)
// keep walking. the first step that lands *behind* the surface — and
// within the thickness band — is a crossing.
// REFINE bisect between the last-in-front point and the crossing point to
// pin the intersection, killing stair-steps.
// DONE   we have a hit (or ran out of steps / left the screen).
//
// everything is in view space; we only project to uv to read the depth buffer.

typedef enum {
    SSRX_MARCH_MISS = 0,   // ran out of steps or left the screen
    SSRX_MARCH_HIT  = 1,   // found and refined a crossing
    SSRX_MARCH_SKY  = 2    // crossed but the surface was the far plane / sky
} ssrx_march_status;

// knobs for a single march. usually filled from the pass tunables.
typedef struct {
    int   max_steps;      // linear steps before giving up
    int   refine_steps;   // bisections after a crossing
    float stride;         // first-step length, view-space units
    float growth;         // geometric stride growth per step
    float thickness;      // view-z band that counts as a hit
    float start_jitter;   // signed sub-stride offset applied to the first step
                          // (see ssr_jitter) to break march banding; 0 = none
} ssrx_march_params;

// the outcome of a march.
typedef struct {
    ssrx_march_status status;
    vec2  hit_uv;         // where the reflection landed (valid if HIT)
    float hit_view_z;     // scene view-z at the hit
    float travel;         // view-space distance marched to the hit
    float t;              // normalized travel 0..1 vs the max march length
    int   steps;          // linear steps actually taken (perf/telemetry)
} ssrx_march_result;

// fill params from the config defaults. caller can then tweak fields.
void ssrx_march_params_default(ssrx_march_params *p);

// run the march. returns the result by value. `ray` must be valid (caller
// checks ray.valid first; an invalid ray yields MISS without touching depth).
ssrx_march_result ssrx_march_run(const ssrx_ray *ray,
                                 const ssrx_gbuffer *g,
                                 const ssrx_depthbuf *depth,
                                 const ssrx_march_params *p);

#endif
