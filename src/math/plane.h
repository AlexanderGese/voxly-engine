#ifndef MATH_PLANE_H
#define MATH_PLANE_H

#include "vec3.h"

// a plane defined by normal + distance from origin.
// signed distance: n.p + d

typedef struct {
    vec3  n;
    float d;
} plane;

plane plane_from_points(vec3 a, vec3 b, vec3 c);
float plane_signed_distance(plane p, vec3 point);
int   plane_ray(plane p, vec3 origin, vec3 dir, float *t_out);

#endif
