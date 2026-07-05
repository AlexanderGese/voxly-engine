#include "decals_box.h"
#include "decals_config.h"

#include <math.h>

// projector box math. all pure, no gl. the box is the unit cube [-0.5,0.5]^3
// scaled by `half`*2 along the basis and parked at `center`. world_to_local
// folds the inverse rotation + inverse translation + inverse scale into one
// matrix so the fragment shader (and the cpu contains() test) is a single mul.

static vec3 v_normalize_safe(vec3 v, vec3 fallback) {
    float len = vec3_length(v);
    if (len < 1e-6f) return fallback;
    return vec3_scale(v, 1.0f / len);
}

// gram-schmidt the up hint against forward, then cross for right. forward is
// taken as authoritative (it's the projection axis, callers care about it).
static void ortho_basis(vec3 fwd, vec3 up_hint, vec3 *right, vec3 *up) {
    // guard against up parallel to fwd, pick a different seed if so
    if (fabsf(vec3_dot(fwd, up_hint)) > 0.999f)
        up_hint = (fabsf(fwd.y) > 0.9f) ? VEC3_FWD : VEC3_UP;

    vec3 r = vec3_normalize(vec3_cross(up_hint, fwd));
    vec3 u = vec3_cross(fwd, r);   // already unit, fwd & r orthonormal
    *right = r;
    *up    = u;
}

decals_projector decals_box_make(vec3 center, vec3 forward, vec3 up_hint, vec3 size) {
    decals_projector p;
    p.center = center;
    p.normal = v_normalize_safe(forward, VEC3_FWD);
    ortho_basis(p.normal, v_normalize_safe(up_hint, VEC3_UP), &p.right, &p.up);

    // clamp half extents into sane range so we never build a degenerate box
    // (zero volume -> the inverse blows up) or a world-eating one.
    float hx = size.x * 0.5f, hy = size.y * 0.5f, hz = size.z * 0.5f;
    if (hx < DECALS_MIN_HALF_EXTENT) hx = DECALS_MIN_HALF_EXTENT;
    if (hy < DECALS_MIN_HALF_EXTENT) hy = DECALS_MIN_HALF_EXTENT;
    if (hz < DECALS_MIN_HALF_EXTENT) hz = DECALS_MIN_HALF_EXTENT;
    if (hx > DECALS_MAX_HALF_EXTENT) hx = DECALS_MAX_HALF_EXTENT;
    if (hy > DECALS_MAX_HALF_EXTENT) hy = DECALS_MAX_HALF_EXTENT;
    if (hz > DECALS_MAX_HALF_EXTENT) hz = DECALS_MAX_HALF_EXTENT;
    p.half = vec3_new(hx, hy, hz);

    decals_box_refresh(&p);
    return p;
}

// build world_to_local. local space is the cube where each axis spans [-0.5,
// 0.5]; so the forward transform is: rotate basis, scale by 2*half, translate
// to center. we want the inverse: (world - center) projected onto each unit
// basis axis, divided by (2*half). that's exactly what the rows below encode.
void decals_box_refresh(decals_projector *p) {
    mat4 m = mat4_identity();
    float sx = 1.0f / (2.0f * p->half.x);
    float sy = 1.0f / (2.0f * p->half.y);
    float sz = 1.0f / (2.0f * p->half.z);

    // rows of the rotation-inverse are the basis vectors (orthonormal => inverse
    // == transpose), each scaled by the inverse extent. column-major storage:
    // m[col][row], so row 0 lives at m[0][0], m[1][0], m[2][0].
    m.m[0][0] = p->right.x * sx;  m.m[1][0] = p->right.y * sx;  m.m[2][0] = p->right.z * sx;
    m.m[0][1] = p->up.x    * sy;  m.m[1][1] = p->up.y    * sy;  m.m[2][1] = p->up.z    * sy;
    m.m[0][2] = p->normal.x* sz;  m.m[1][2] = p->normal.y* sz;  m.m[2][2] = p->normal.z* sz;

    // translation: -(R^-1 * center) folded into the last column.
    vec3 c = p->center;
    m.m[3][0] = -(p->right.x  * c.x + p->right.y  * c.y + p->right.z  * c.z) * sx;
    m.m[3][1] = -(p->up.x     * c.x + p->up.y     * c.y + p->up.z     * c.z) * sy;
    m.m[3][2] = -(p->normal.x * c.x + p->normal.y * c.y + p->normal.z * c.z) * sz;
    p->world_to_local = m;

    // world aabb: sum of absolute basis*half projections onto each world axis.
    vec3 ex = vec3_scale(p->right,  p->half.x);
    vec3 ey = vec3_scale(p->up,     p->half.y);
    vec3 ez = vec3_scale(p->normal, p->half.z);
    vec3 r = vec3_new(fabsf(ex.x) + fabsf(ey.x) + fabsf(ez.x),
                      fabsf(ex.y) + fabsf(ey.y) + fabsf(ez.y),
                      fabsf(ex.z) + fabsf(ey.z) + fabsf(ez.z));
    p->bounds = aabb_make(vec3_sub(p->center, r), vec3_add(p->center, r));
}

vec3 decals_box_to_local(const decals_projector *p, vec3 world) {
    return mat4_mul_vec3(p->world_to_local, world);
}

int decals_box_contains(const decals_projector *p, vec3 world) {
    vec3 l = decals_box_to_local(p, world);
    return l.x >= -0.5f && l.x <= 0.5f &&
           l.y >= -0.5f && l.y <= 0.5f &&
           l.z >= -0.5f && l.z <= 0.5f;
}

void decals_box_local_uv(const decals_atlas_region *r, float lx, float ly,
                         float *u, float *v) {
    // map [-0.5,0.5] -> [0,1], clamp the slop at the edges, then lerp into the
    // atlas rect. y is flipped so the decal art isn't upside down (gl uv origin
    // is bottom-left, our art is authored top-left).
    float fx = lx + 0.5f, fy = ly + 0.5f;
    if (fx < 0.0f) fx = 0.0f; else if (fx > 1.0f) fx = 1.0f;
    if (fy < 0.0f) fy = 0.0f; else if (fy > 1.0f) fy = 1.0f;
    fy = 1.0f - fy;
    *u = r->uv0[0] + (r->uv1[0] - r->uv0[0]) * fx;
    *v = r->uv0[1] + (r->uv1[1] - r->uv0[1]) * fy;
}
