#include "plane.h"

plane plane_from_points(vec3 a, vec3 b, vec3 c) {
    vec3 u = vec3_sub(b, a);
    vec3 v = vec3_sub(c, a);
    vec3 n = vec3_normalize(vec3_cross(u, v));
    plane p = { n, -vec3_dot(n, a) };
    return p;
}

float plane_signed_distance(plane p, vec3 point) {
    return vec3_dot(p.n, point) + p.d;
}

int plane_ray(plane p, vec3 origin, vec3 dir, float *t_out) {
    float denom = vec3_dot(p.n, dir);
    if (denom > -1e-6f && denom < 1e-6f) return 0;
    float t = -(vec3_dot(p.n, origin) + p.d) / denom;
    if (t < 0) return 0;
    if (t_out) *t_out = t;
    return 1;
}
