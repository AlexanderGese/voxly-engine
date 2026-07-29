#include "behavior_flee.h"
#include "mob_util.h"

#ifndef VOXL_PI
#define VOXL_PI 3.14159265358979323846f
#endif

float voxl_mob_flee_tick(voxl_mob *m, vec3 threat, float speed, float dt) {
    // the away direction is just "face the threat then add pi".
    float toward = voxl_mob_yaw_to(m->pos, threat);
    float want = voxl_mob_wrap_angle(toward + VOXL_PI);
    // panicked turning is snappy.
    m->yaw = voxl_mob_turn_toward(m->yaw, want, 9.0f * dt);
    voxl_mob_walk_forward(m, speed, dt);
    return voxl_mob_flat_dist(m->pos, threat);
}

bool voxl_mob_flee_is_safe(const voxl_mob *m, vec3 threat, float safe) {
    return voxl_mob_flat_dist_sq(m->pos, threat) >= safe * safe;
}
