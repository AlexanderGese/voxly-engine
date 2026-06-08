#include "bvh_box.h"

#include <math.h>
#include <float.h>

aabb bvh_box_empty(void) {
    // inverted box. any union with a real box snaps it to that box. the slab
    // raycast would do nonsense on this, so never hand one to a query - it's a
    // builder seed only.
    vec3 lo = { FLT_MAX,  FLT_MAX,  FLT_MAX };
    vec3 hi = { -FLT_MAX, -FLT_MAX, -FLT_MAX };
    return (aabb){ lo, hi };
}

aabb bvh_box_union(aabb a, aabb b) {
    aabb r;
    r.min.x = fminf(a.min.x, b.min.x);
    r.min.y = fminf(a.min.y, b.min.y);
    r.min.z = fminf(a.min.z, b.min.z);
    r.max.x = fmaxf(a.max.x, b.max.x);
    r.max.y = fmaxf(a.max.y, b.max.y);
    r.max.z = fmaxf(a.max.z, b.max.z);
    return r;
}

aabb bvh_box_include(aabb a, vec3 p) {
    a.min.x = fminf(a.min.x, p.x);
    a.min.y = fminf(a.min.y, p.y);
    a.min.z = fminf(a.min.z, p.z);
    a.max.x = fmaxf(a.max.x, p.x);
    a.max.y = fmaxf(a.max.y, p.y);
    a.max.z = fmaxf(a.max.z, p.z);
    return a;
}

vec3 bvh_box_centroid(aabb a) {
    return (vec3){
        (a.min.x + a.max.x) * 0.5f,
        (a.min.y + a.max.y) * 0.5f,
        (a.min.z + a.max.z) * 0.5f,
    };
}

vec3 bvh_box_half(aabb a) {
    return (vec3){
        (a.max.x - a.min.x) * 0.5f,
        (a.max.y - a.min.y) * 0.5f,
        (a.max.z - a.min.z) * 0.5f,
    };
}

float bvh_box_area(aabb a) {
    float dx = a.max.x - a.min.x;
    float dy = a.max.y - a.min.y;
    float dz = a.max.z - a.min.z;
    // empty/inverted box: any negative extent means "no area". clamp so the SAH
    // never sees a negative term.
    if (dx < 0 || dy < 0 || dz < 0) return 0.0f;
    return 2.0f * (dx * dy + dy * dz + dz * dx);
}

int bvh_box_max_axis(aabb a) {
    float dx = a.max.x - a.min.x;
    float dy = a.max.y - a.min.y;
    float dz = a.max.z - a.min.z;
    if (dx >= dy && dx >= dz) return 0;
    if (dy >= dz) return 1;
    return 2;
}

int bvh_box_contains_box(aabb outer, aabb inner) {
    return inner.min.x >= outer.min.x && inner.max.x <= outer.max.x
        && inner.min.y >= outer.min.y && inner.max.y <= outer.max.y
        && inner.min.z >= outer.min.z && inner.max.z <= outer.max.z;
}

float bvh_box_overlap_volume(aabb a, aabb b) {
    float ox = fminf(a.max.x, b.max.x) - fmaxf(a.min.x, b.min.x);
    float oy = fminf(a.max.y, b.max.y) - fmaxf(a.min.y, b.min.y);
    float oz = fminf(a.max.z, b.max.z) - fmaxf(a.min.z, b.min.z);
    if (ox <= 0 || oy <= 0 || oz <= 0) return 0.0f;  // no overlap on some axis
    return ox * oy * oz;
}

aabb bvh_box_pad(aabb a, float margin) {
    a.min.x -= margin; a.min.y -= margin; a.min.z -= margin;
    a.max.x += margin; a.max.y += margin; a.max.z += margin;
    return a;
}
