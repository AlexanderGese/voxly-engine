#ifndef MATH_VEC2_H
#define MATH_VEC2_H

typedef struct { float x, y; } vec2;

vec2  vec2_new(float x, float y);
vec2  vec2_add(vec2 a, vec2 b);
vec2  vec2_sub(vec2 a, vec2 b);
vec2  vec2_scale(vec2 a, float s);
float vec2_dot(vec2 a, vec2 b);
float vec2_length(vec2 a);
vec2  vec2_normalize(vec2 a);
vec2  vec2_lerp(vec2 a, vec2 b, float t);

#endif
