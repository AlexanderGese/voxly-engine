#include "ephysics_aabb.h"
#include <math.h>

aabb ephysics_body_box(const ephys_body *b) {
    return ephysics_box_at(b, b->pos);
}

aabb ephysics_box_at(const ephys_body *b, vec3 feet) {
    // pos is feet-center. center of box sits at feet.y + center_y.
    vec3 c = vec3_new(feet.x, feet.y + b->center_y, feet.z);
    aabb out;
    out.min = vec3_sub(c, b->half);
    out.max = vec3_add(c, b->half);
    return out;
}

aabb ephysics_minkowski(aabb a, vec3 half) {
    aabb out;
    out.min = vec3_sub(a.min, half);
    out.max = vec3_add(a.max, half);
    return out;
}

vec3 ephysics_overlap_depth(aabb a, aabb b) {
    // for each axis, distance you'd have to move b out of a (smaller of the two
    // sides). sign tells you which way is cheaper to escape.
    vec3 d;
    float ax_pos = a.max.x - b.min.x;   // push b in +x to clear a
    float ax_neg = b.max.x - a.min.x;   // push b in -x
    d.x = (ax_pos < ax_neg) ? ax_pos : -ax_neg;

    float ay_pos = a.max.y - b.min.y;
    float ay_neg = b.max.y - a.min.y;
    d.y = (ay_pos < ay_neg) ? ay_pos : -ay_neg;

    float az_pos = a.max.z - b.min.z;
    float az_neg = b.max.z - a.min.z;
    d.z = (az_pos < az_neg) ? az_pos : -az_neg;
    return d;
}

int ephysics_touching(aabb a, aabb b, float eps) {
    return (a.min.x - eps <= b.max.x && a.max.x + eps >= b.min.x) &&
           (a.min.y - eps <= b.max.y && a.max.y + eps >= b.min.y) &&
           (a.min.z - eps <= b.max.z && a.max.z + eps >= b.min.z);
}
