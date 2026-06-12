#ifndef MATH_MAT4_H
#define MATH_MAT4_H
#include "vec3.h"
typedef struct {
    float m[4][4];
} mat4;
mat4 mat4_identity(void);
mat4 mat4_zero(void);
mat4 mat4_mul(mat4 a, mat4 b);
vec3 mat4_mul_vec3(mat4 m, vec3 v);
#endif
