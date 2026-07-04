#ifndef RENDER_CULL_FRUSTUM_H
#define RENDER_CULL_FRUSTUM_H

#include "../../math/mat4.h"
#include "../../math/aabb.h"
#include "../../math/vec3.h"

// frustum culling, the cull-subsystem flavour of it.
//
// the engine already has math/frustum.h with a plane-soup test, but that
// one only does the conservative all-corners-out check and i wanted the
// p/n-vertex trick plus a cheap sphere reject in front of it. so this is
// a parallel little frustum that we own. same plane convention:
// point inside when n.p + d >= 0.

enum {
    CFRUS_LEFT = 0,
    CFRUS_RIGHT,
    CFRUS_BOTTOM,
    CFRUS_TOP,
    CFRUS_NEAR,
    CFRUS_FAR,
    CFRUS_PLANES
};

typedef struct {
    // (nx, ny, nz, d) per plane, normals point inward, normalized.
    float p[CFRUS_PLANES][4];
    vec3  origin;   // camera-ish origin, cached for distance ordering
} cull_frustum;

// classification result for an aabb vs the frustum.
enum {
    CFRUS_OUTSIDE = 0,
    CFRUS_INTERSECT,
    CFRUS_INSIDE       // fully contained, children dont need testing
};

// extract from a view-projection matrix. normalizes the planes so the
// signed distance is metric (lets us do sphere tests properly).
void cull_frustum_from_vp(cull_frustum *f, mat4 vp, vec3 cam_pos);

// cheap pre-test: is a bounding sphere at all touching the frustum?
// good as a first reject before the 6-plane aabb walk.
int  cull_frustum_sphere(const cull_frustum *f, vec3 c, float r);

// full aabb classify using p-vertex / n-vertex. returns CFRUS_*.
int  cull_frustum_aabb(const cull_frustum *f, aabb a);

// boolean shorthand for "should i bother drawing this box".
static inline int cull_frustum_visible(const cull_frustum *f, aabb a) {
    return cull_frustum_aabb(f, a) != CFRUS_OUTSIDE;
}

#endif
