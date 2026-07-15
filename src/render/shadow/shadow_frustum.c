#include "shadow_frustum.h"

#include <math.h>

// pull the camera basis + eye back out of the (column-major) view matrix.
// view = R^T * T(-eye), so the rotation rows are the world-space basis and
// eye = -R * translation_column. standard inverse-of-rigid-transform trick.
static void decompose_view(mat4 v, vec3 *right, vec3 *up, vec3 *fwd, vec3 *eye) {
    // rows of the 3x3 rotation = world basis (right, up, -forward)
    right->x = v.m[0][0]; right->y = v.m[1][0]; right->z = v.m[2][0];
    up->x    = v.m[0][1]; up->y    = v.m[1][1]; up->z    = v.m[2][1];
    // third row is +forward in view space which is -view direction in world
    vec3 back = { v.m[0][2], v.m[1][2], v.m[2][2] };
    *fwd = vec3_neg(back);

    // translation column of the view matrix
    vec3 t = { v.m[3][0], v.m[3][1], v.m[3][2] };
    // eye = -(R^T)^-1 t = -R t, with R rows being right/up/back
    eye->x = -(right->x * t.x + up->x * t.y + back.x * t.z);
    eye->y = -(right->y * t.x + up->y * t.y + back.y * t.z);
    eye->z = -(right->z * t.x + up->z * t.y + back.z * t.z);
}

void shadow_frustum_corners(shadow_corners *out,
                            mat4 cam_view, float fov_rad, float aspect,
                            float near_d, float far_d) {
    vec3 right, up, fwd, eye;
    decompose_view(cam_view, &right, &up, &fwd, &eye);

    // half-extents of the near/far slice planes from the vertical fov
    float tan_v = tanf(fov_rad * 0.5f);
    float nh = near_d * tan_v;          // near half height
    float nw = nh * aspect;             // near half width
    float fh = far_d * tan_v;
    float fw = fh * aspect;

    vec3 nc = vec3_add(eye, vec3_scale(fwd, near_d)); // near plane center
    vec3 fc = vec3_add(eye, vec3_scale(fwd, far_d));  // far plane center

    vec3 ru_n_w = vec3_scale(right, nw);
    vec3 up_n_h = vec3_scale(up, nh);
    vec3 ru_f_w = vec3_scale(right, fw);
    vec3 up_f_h = vec3_scale(up, fh);

    // near: bl, br, tr, tl
    out->c[0] = vec3_sub(vec3_sub(nc, ru_n_w), up_n_h);
    out->c[1] = vec3_sub(vec3_add(nc, ru_n_w), up_n_h);
    out->c[2] = vec3_add(vec3_add(nc, ru_n_w), up_n_h);
    out->c[3] = vec3_add(vec3_sub(nc, ru_n_w), up_n_h);
    // far
    out->c[4] = vec3_sub(vec3_sub(fc, ru_f_w), up_f_h);
    out->c[5] = vec3_sub(vec3_add(fc, ru_f_w), up_f_h);
    out->c[6] = vec3_add(vec3_add(fc, ru_f_w), up_f_h);
    out->c[7] = vec3_add(vec3_sub(fc, ru_f_w), up_f_h);
}

vec3 shadow_frustum_center(const shadow_corners *fc) {
    vec3 sum = VEC3_ZERO;
    for (int i = 0; i < 8; i++) sum = vec3_add(sum, fc->c[i]);
    return vec3_scale(sum, 1.0f / 8.0f);
}

float shadow_frustum_radius(const shadow_corners *fc, vec3 center) {
    float r2 = 0.0f;
    for (int i = 0; i < 8; i++) {
        float d = vec3_length_sq(vec3_sub(fc->c[i], center));
        if (d > r2) r2 = d;
    }
    return sqrtf(r2);
}
