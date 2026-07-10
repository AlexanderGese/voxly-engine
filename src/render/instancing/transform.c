#include "transform.h"

#include <string.h>

// compose the trs matrix. we lean on the engine's mat4 helpers rather than
// hand-rolling the multiply — they're already column-major and tested.
mat4 instancing_xform_model(const instancing_instance *it) {
    mat4 s = mat4_scale(it->scale);
    mat4 rx = mat4_rotate_x(it->euler.x);
    mat4 ry = mat4_rotate_y(it->euler.y);
    mat4 rz = mat4_rotate_z(it->euler.z);
    mat4 t = mat4_translate(it->position);

    // r = rz * ry * rx  (apply x first, then y, then z)
    mat4 r = mat4_mul(rz, mat4_mul(ry, rx));
    // model = t * r * s
    return mat4_mul(t, mat4_mul(r, s));
}

void instancing_xform_bake(const instancing_instance *it,
                               instancing_gpu_instance *out) {
    mat4 model = instancing_xform_model(it);

    // mat4 stores m[col][row]; that's exactly the 16-float column-major run
    // gl expects, so a flat copy is correct. no transpose nonsense.
    memcpy(out->model, mat4_data(&model), sizeof out->model);

    out->tint[0] = it->tint[0];
    out->tint[1] = it->tint[1];
    out->tint[2] = it->tint[2];
    out->tint[3] = it->tint[3];
    out->light = it->light;
    out->_pad[0] = out->_pad[1] = out->_pad[2] = 0.0f;
}

aabb instancing_xform_world_box(const instancing_instance *it) {
    mat4 model = instancing_xform_model(it);

    // transform all eight corners and accumulate the min/max. rotation means
    // we can't just transform min/max directly — that gives a sheared box.
    aabb lb = it->local_box;
    float xs[2] = { lb.min.x, lb.max.x };
    float ys[2] = { lb.min.y, lb.max.y };
    float zs[2] = { lb.min.z, lb.max.z };

    vec3 mn = { 1e30f, 1e30f, 1e30f };
    vec3 mx = { -1e30f, -1e30f, -1e30f };

    for (int i = 0; i < 2; ++i)
    for (int j = 0; j < 2; ++j)
    for (int k = 0; k < 2; ++k) {
        vec3 corner = { xs[i], ys[j], zs[k] };
        vec3 w = mat4_mul_vec3(model, corner);
        mn = vec3_min(mn, w);
        mx = vec3_max(mx, w);
    }
    return aabb_make(mn, mx);
}
