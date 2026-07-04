#ifndef RENDER_CULL_SPHERE_H
#define RENDER_CULL_SPHERE_H

#include "../../math/vec3.h"
#include "../../math/aabb.h"
#include "frustum_cull.h"

// bounding-sphere helpers. the per-chunk aabb test is exact-ish but the
// sphere test is cheaper, so the orchestrator can do a sphere broad-phase
// over a whole ring of chunks before bothering with boxes. also used for
// the spherical render-distance falloff (a box distance cutoff makes the
// corners of the render area pop differently than the sides, which looks
// off — a sphere is rounder, pun intended).

typedef struct {
    vec3  center;
    float radius;
} cull_sphere;

// sphere that tightly wraps an aabb.
cull_sphere cull_sphere_of_aabb(aabb a);

// is point p within radius of the sphere center? squared, no sqrt.
int  cull_sphere_contains(cull_sphere s, vec3 p);

// do two spheres overlap?
int  cull_sphere_overlap(cull_sphere a, cull_sphere b);

// frustum broad-phase: 1 if the sphere is at all inside the frustum.
int  cull_sphere_in_frustum(const cull_frustum *f, cull_sphere s);

// spherical distance cutoff: is the chunk center within render_radius
// (world units) of the camera? cheaper round falloff than the box cutoff.
int  cull_sphere_in_range(vec3 cam, vec3 chunk_center, float render_radius);

#endif
