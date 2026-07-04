#include "sphere_cull.h"

#include <math.h>

cull_sphere cull_sphere_of_aabb(aabb a) {
    cull_sphere s;
    s.center = (vec3){
        (a.min.x + a.max.x) * 0.5f,
        (a.min.y + a.max.y) * 0.5f,
        (a.min.z + a.max.z) * 0.5f
    };
    // radius = half the diagonal.
    float hx = (a.max.x - a.min.x) * 0.5f;
    float hy = (a.max.y - a.min.y) * 0.5f;
    float hz = (a.max.z - a.min.z) * 0.5f;
    s.radius = sqrtf(hx*hx + hy*hy + hz*hz);
    return s;
}

int cull_sphere_contains(cull_sphere s, vec3 p) {
    float dx = p.x - s.center.x;
    float dy = p.y - s.center.y;
    float dz = p.z - s.center.z;
    return (dx*dx + dy*dy + dz*dz) <= (s.radius * s.radius);
}

int cull_sphere_overlap(cull_sphere a, cull_sphere b) {
    float dx = a.center.x - b.center.x;
    float dy = a.center.y - b.center.y;
    float dz = a.center.z - b.center.z;
    float rr = a.radius + b.radius;
    return (dx*dx + dy*dy + dz*dz) <= (rr * rr);
}

int cull_sphere_in_frustum(const cull_frustum *f, cull_sphere s) {
    return cull_frustum_sphere(f, s.center, s.radius);
}

int cull_sphere_in_range(vec3 cam, vec3 chunk_center, float render_radius) {
    // ignore vertical: render distance is horizontal in a voxel game,
    // otherwise looking up/down would unload the ground under you.
    float dx = cam.x - chunk_center.x;
    float dz = cam.z - chunk_center.z;
    return (dx*dx + dz*dz) <= (render_radius * render_radius);
}
