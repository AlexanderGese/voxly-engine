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
}

float voxl_mob_turn_toward(float cur, float want, float max_step) {
    float diff = voxl_mob_wrap_angle(want - cur);
    if (diff >  max_step) diff =  max_step;
    if (diff < -max_step) diff = -max_step;
    return voxl_mob_wrap_angle(cur + diff);
}

vec3 voxl_mob_heading(float yaw) {
    vec3 h;
    h.x =  sinf(yaw);
    h.y =  0.0f;
    h.z = -cosf(yaw);
    return h;
}

void voxl_mob_walk_forward(voxl_mob *m, float speed, float dt) {
    vec3 h = voxl_mob_heading(m->yaw);
    m->pos.x += h.x * speed * dt;
    m->pos.z += h.z * speed * dt;
}

float voxl_mob_clampf(float v, float lo, float hi) {
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}

bool voxl_mob_is_hostile(voxl_mob_kind k) {
    return k == VOXL_MOB_ZOMBIE  ||
           k == VOXL_MOB_SKELETON ||
           k == VOXL_MOB_CREEPER ||
           k == VOXL_MOB_SPIDER;
}

bool voxl_mob_is_passive(voxl_mob_kind k) {
    return k == VOXL_MOB_COW   ||
           k == VOXL_MOB_PIG   ||
           k == VOXL_MOB_CHICKEN ||
           k == VOXL_MOB_SHEEP;
}
