#ifndef RENDER_SSR_RAY_H
#define RENDER_SSR_RAY_H

#include "../../math/vec3.h"

// a reflected ray in view space. origin sits on the reflective surface, dir is
// the reflected view direction (normalized). we keep the surface normal around
// because the march wants it for the thickness test and the resolve wants it
// for fresnel.

typedef struct {
    vec3  origin;     // view-space surface point
    vec3  dir;        // normalized reflected direction
    vec3  normal;     // view-space surface normal (normalized)
    vec3  view;       // normalized view->surface direction (from eye)
    float n_dot_v;    // dot(normal, -view), clamped [0,1] — for fresnel
    int   valid;      // 0 if the ray was culled at build time
} ssrx_ray;

// reflect an incident view-space direction about a normal. r = i - 2(i·n)n.
// both inputs expected normalized; result is normalized.
vec3 ssrx_reflect(vec3 incident, vec3 normal);

// build the reflected ray for a surface fragment.
// view_pos : view-space position of the fragment (origin)
// normal   : view-space normal (need not be pre-normalized)
// the eye is at the origin in view space, so the incident direction is just
// normalize(view_pos). marks the ray invalid if the reflection points back at
// the camera hard enough to be useless.
ssrx_ray ssrx_ray_make(vec3 view_pos, vec3 normal);

// advance a point along the ray by `dist` view-space units. trivial, but it
// keeps the march loop readable and gives one place to instrument.
vec3 ssrx_ray_at(const ssrx_ray *r, float dist);

#endif
