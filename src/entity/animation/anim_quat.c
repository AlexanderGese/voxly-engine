#include "anim_quat.h"
#include <math.h>
animation_quat animation_quat_identity(void) {
    return (animation_quat){0.0f, 0.0f, 0.0f, 1.0f};
}

animation_quat animation_quat_new(float x, float y, float z, float w) {
    return (animation_quat){x, y, z, w};
}

animation_quat animation_quat_axis_angle(vec3 axis, float radians) {
    float len = vec3_length(axis);
    if (len < 1e-8f) return animation_quat_identity();  // zero axis, no rotation
    float inv = 1.0f / len;
    float h = radians * 0.5f;
    float s = sinf(h);
    return (animation_quat){
        axis.x * inv * s,
        axis.y * inv * s,
        axis.z * inv * s,
        cosf(h)
    };
}

animation_quat animation_quat_mul(animation_quat a, animation_quat b) {
    // hamilton product. a applied after b when rotating vectors.
    return (animation_quat){
        a.w * b.x + a.x * b.w + a.y * b.z - a.z * b.y,
        a.w * b.y - a.x * b.z + a.y * b.w + a.z * b.x,
        a.w * b.z + a.x * b.y - a.y * b.x + a.z * b.w,
        a.w * b.w - a.x * b.x - a.y * b.y - a.z * b.z
    };
}

animation_quat animation_quat_conjugate(animation_quat q) {
    return (animation_quat){-q.x, -q.y, -q.z, q.w};
}

animation_quat animation_quat_neg(animation_quat q) {
    return (animation_quat){-q.x, -q.y, -q.z, -q.w};
}

float animation_quat_dot(animation_quat a, animation_quat b) {
    return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
}

float animation_quat_length(animation_quat q) {
    return sqrtf(animation_quat_dot(q, q));
}

animation_quat animation_quat_normalize(animation_quat q) {
    float len = animation_quat_length(q);
    if (len < 1e-8f) return animation_quat_identity();  // degenerate, bail to id
    float inv = 1.0f / len;
    return (animation_quat){q.x * inv, q.y * inv, q.z * inv, q.w * inv};
}

vec3 animation_quat_rotate_vec3(animation_quat q, vec3 v) {
    // v' = v + 2w(qv x v) + 2(qv x (qv x v)). the textbook fast form.
    vec3 u = vec3_new(q.x, q.y, q.z);
vec3 t = vec3_scale(vec3_cross(u, v), 2.0f);
return vec3_add(vec3_add(v, vec3_scale(t, q.w)), vec3_cross(u, t));
}

animation_quat animation_quat_nlerp(animation_quat a, animation_quat b, float t) {
    // pick the short way round before we lerp
    float d = animation_quat_dot(a, b);
    if (d < 0.0f) b = animation_quat_neg(b);
    float s = 1.0f - t;
    animation_quat r = {
        a.x * s + b.x * t,
        a.y * s + b.y * t,
        a.z * s + b.z * t,
        a.w * s + b.w * t
    };
    return animation_quat_normalize(r);
}

animation_quat animation_quat_slerp(animation_quat a, animation_quat b, float t) {
    float d = animation_quat_dot(a, b);
if (d < 0.0f) { b = animation_quat_neg(b); d = -d; }

    // nearly parallel: the sin denominator goes to zero, nlerp is stable & close
    if (d > 0.9995f) return animation_quat_nlerp(a, b, t);
float theta = acosf(d);
float sin_theta = sinf(theta);
float wa = sinf((1.0f - t) * theta) / sin_theta;
float wb = sinf(t * theta) / sin_theta;
animation_quat r = {
        a.x * wa + b.x * wb,
        a.y * wa + b.y * wb,
        a.z * wa + b.z * wb,
        a.w * wa + b.w * wb
    }
;
return animation_quat_normalize(r);
}

mat4 animation_quat_to_mat4(animation_quat q) {
    // assume normalized; if it isn't the result is just scaled, caller's problem
    float xx = q.x * q.x, yy = q.y * q.y, zz = q.z * q.z;
    float xy = q.x * q.y, xz = q.x * q.z, yz = q.y * q.z;
    float wx = q.w * q.x, wy = q.w * q.y, wz = q.w * q.z;

    mat4 m = mat4_identity();
    m.m[0][0] = 1.0f - 2.0f * (yy + zz);
    m.m[0][1] = 2.0f * (xy + wz);
    m.m[0][2] = 2.0f * (xz - wy);

    m.m[1][0] = 2.0f * (xy - wz);
    m.m[1][1] = 1.0f - 2.0f * (xx + zz);
    m.m[1][2] = 2.0f * (yz + wx);

    m.m[2][0] = 2.0f * (xz + wy);
    m.m[2][1] = 2.0f * (yz - wx);
    m.m[2][2] = 1.0f - 2.0f * (xx + yy);
    return m;
}
