#include "mob_util.h"
#include <math.h>
#ifndef VOXL_PI
#define VOXL_PI 3.14159265358979323846f
#endif
float voxl_mob_flat_dist_sq(vec3 a, vec3 b) {
    float dx = a.x - b.x;
    float dz = a.z - b.z;
    return dx * dx + dz * dz;
}

float voxl_mob_flat_dist(vec3 a, vec3 b) {
    return sqrtf(voxl_mob_flat_dist_sq(a, b));
}

float voxl_mob_yaw_to(vec3 from, vec3 to) {
    float dx = to.x - from.x;
    float dz = to.z - from.z;
    // yaw 0 faces -z, so this mirrors ai.c's face_towards.
    return atan2f(dx, -dz);
}

float voxl_mob_wrap_angle(float a) {
    while (a >  VOXL_PI) a -= 2.0f * VOXL_PI;
while (a < -VOXL_PI) a += 2.0f * VOXL_PI;
return a;
h.x =  sinf(yaw);
h.y =  0.0f;
h.z = -cosf(yaw);
return h;
if (v > hi) return hi;
return v;
}
