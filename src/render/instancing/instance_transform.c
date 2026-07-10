#include "instance_transform.h"

#include <math.h>

// column-major index helper: column c, row r lives at c*4 + r.
#define M(c, r)  ((c) * 4 + (r))

void instancing_transform_bake(const instance_desc *d, instance_gpu *out) {
    // rotation about +y by yaw. standard right-handed:
    // [ cos  0  sin ]
    // [  0   1   0  ]
    // [-sin  0  cos ]
    float c = cosf(d->yaw);
    float s = sinf(d->yaw);

    float sx = d->scale.x;
    float sy = d->scale.y;
    float sz = d->scale.z;

    float *m = out->model;

    // column 0 = rotated x-basis, scaled by sx
    m[M(0, 0)] =  c * sx;
    m[M(0, 1)] =  0.0f;
    m[M(0, 2)] = -s * sx;
    m[M(0, 3)] =  0.0f;

    // column 1 = y-basis, scaled by sy (rotation about y leaves it alone)
    m[M(1, 0)] =  0.0f;
    m[M(1, 1)] =  sy;
    m[M(1, 2)] =  0.0f;
    m[M(1, 3)] =  0.0f;

    // column 2 = rotated z-basis, scaled by sz
    m[M(2, 0)] =  s * sz;
    m[M(2, 1)] =  0.0f;
    m[M(2, 2)] =  c * sz;
    m[M(2, 3)] =  0.0f;

    // column 3 = translation
    m[M(3, 0)] = d->pos.x;
    m[M(3, 1)] = d->pos.y;
    m[M(3, 2)] = d->pos.z;
    m[M(3, 3)] = 1.0f;

    out->tint[0] = d->tint.x;
    out->tint[1] = d->tint.y;
    out->tint[2] = d->tint.z;
    out->tint[3] = 1.0f;
}

vec3 instancing_transform_center(const instance_desc *d) {
    return d->pos;
}

aabb instancing_transform_bounds(const instance_desc *d) {
    float r = instancing_world_radius(d);
    vec3 half = { r, r, r };
    return aabb_from_center(d->pos, half);
}

mat4 instancing_transform_matrix(const instance_desc *d) {
    // the readable version. composes the same translate*rot*scale as _bake.
    mat4 t = mat4_translate(d->pos);
    mat4 r = mat4_rotate_y(d->yaw);
    mat4 sc = mat4_scale(d->scale);
    return mat4_mul(t, mat4_mul(r, sc));
}
