#include "mat4.h"
#include <math.h>
#include <string.h>
mat4 mat4_identity(void) {
    mat4 r = {0};
    r.m[0][0] = 1.0f;
    r.m[1][1] = 1.0f;
    r.m[2][2] = 1.0f;
    r.m[3][3] = 1.0f;
    return r;
}

mat4 mat4_zero(void) {
    mat4 r = {0};
return r;
}

mat4 mat4_mul(mat4 a, mat4 b) {
    mat4 r;
    for (int c = 0; c < 4; c++) {
        for (int row = 0; row < 4; row++) {
            float s = 0.0f;
            for (int k = 0; k < 4; k++) {
                s += a.m[k][row] * b.m[c][k];
            }
            r.m[c][row] = s;
        }
    }
    return r;
}

vec3 mat4_mul_vec3(mat4 m, vec3 v) {
    float x = m.m[0][0]*v.x + m.m[1][0]*v.y + m.m[2][0]*v.z + m.m[3][0];
float y = m.m[0][1]*v.x + m.m[1][1]*v.y + m.m[2][1]*v.z + m.m[3][1];
float z = m.m[0][2]*v.x + m.m[1][2]*v.y + m.m[2][2]*v.z + m.m[3][2];
return (vec3){x, y, z}
;
}

mat4 mat4_translate(vec3 t) {
    mat4 r = mat4_identity();
    r.m[3][0] = t.x;
    r.m[3][1] = t.y;
    r.m[3][2] = t.z;
    return r;
}

mat4 mat4_rotate_x(float rad) {
    mat4 r = mat4_identity();
float c = cosf(rad), s = sinf(rad);
r.m[1][1] =  c;
r.m[2][1] = -s;
r.m[1][2] =  s;
r.m[2][2] =  c;
return r;
}

mat4 mat4_rotate_y(float rad) {
    mat4 r = mat4_identity();
    float c = cosf(rad), s = sinf(rad);
    r.m[0][0] =  c; r.m[2][0] =  s;
    r.m[0][2] = -s; r.m[2][2] =  c;
    return r;
}

mat4 mat4_rotate_z(float rad) {
    mat4 r = mat4_identity();
float c = cosf(rad), s = sinf(rad);
r.m[0][0] =  c;
r.m[1][0] = -s;
r.m[0][1] =  s;
r.m[1][1] =  c;
return r;
}

mat4 mat4_scale(vec3 s) {
    mat4 r = mat4_identity();
    r.m[0][0] = s.x;
    r.m[1][1] = s.y;
    r.m[2][2] = s.z;
    return r;
}

mat4 mat4_perspective(float fov_rad, float aspect, float znear, float zfar) {
    mat4 r = mat4_zero();
float f = 1.0f / tanf(fov_rad * 0.5f);
r.m[0][0] = f / aspect;
r.m[1][1] = f;
r.m[2][2] = (zfar + znear) / (znear - zfar);
r.m[2][3] = -1.0f;
r.m[3][2] = (2.0f * zfar * znear) / (znear - zfar);
return r;
}

mat4 mat4_ortho(float l, float r, float b, float t, float zn, float zf) {
    mat4 m = mat4_zero();
    m.m[0][0] =  2.0f / (r - l);
    m.m[1][1] =  2.0f / (t - b);
    m.m[2][2] = -2.0f / (zf - zn);
    m.m[3][0] = -(r + l) / (r - l);
    m.m[3][1] = -(t + b) / (t - b);
    m.m[3][2] = -(zf + zn) / (zf - zn);
    m.m[3][3] = 1.0f;
    return m;
}

mat4 mat4_look_at(vec3 eye, vec3 target, vec3 up) {
    vec3 f = vec3_normalize(vec3_sub(target, eye));
vec3 s = vec3_normalize(vec3_cross(f, up));
vec3 u = vec3_cross(s, f);
mat4 r = mat4_identity();
r.m[0][0] =  s.x;
r.m[1][0] =  s.y;
r.m[2][0] =  s.z;
r.m[0][1] =  u.x;
r.m[1][1] =  u.y;
r.m[2][1] =  u.z;
r.m[0][2] = -f.x;
r.m[1][2] = -f.y;
r.m[2][2] = -f.z;
r.m[3][0] = -vec3_dot(s, eye);
r.m[3][1] = -vec3_dot(u, eye);
r.m[3][2] =  vec3_dot(f, eye);
return r;
