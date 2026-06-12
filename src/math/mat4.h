#ifndef MATH_MAT4_H
#define MATH_MAT4_H

#include "vec3.h"

// 4x4 matrix, column major (opengl style)
// layout: m[col][row] — m[0] is the first column (x basis)

typedef struct {
    float m[4][4];
} mat4;

mat4 mat4_identity(void);
mat4 mat4_zero(void);

mat4 mat4_mul(mat4 a, mat4 b);
vec3 mat4_mul_vec3(mat4 m, vec3 v); // assumes w=1

mat4 mat4_translate(vec3 t);
mat4 mat4_rotate_x(float rad);
mat4 mat4_rotate_y(float rad);
mat4 mat4_rotate_z(float rad);
mat4 mat4_scale(vec3 s);

mat4 mat4_perspective(float fov_rad, float aspect, float znear, float zfar);
mat4 mat4_ortho(float l, float r, float b, float t, float zn, float zf);
mat4 mat4_look_at(vec3 eye, vec3 target, vec3 up);

// flat float pointer for glUniformMatrix4fv
const float *mat4_data(const mat4 *m);

#endif
