#include "behavior_chase.h"
#include "mob_util.h"

float voxl_mob_chase_tick(voxl_mob *m, vec3 target, float speed, float dt) {
    float want = voxl_mob_yaw_to(m->pos, target);
    // chasing mobs turn faster than wandering ones.
    m->yaw = voxl_mob_turn_toward(m->yaw, want, 7.0f * dt);
    voxl_mob_walk_forward(m, speed, dt);
    return voxl_mob_flat_dist(m->pos, target);
}

bool voxl_mob_chase_should_give_up(const voxl_mob *m, vec3 target, float lose) {
    return voxl_mob_flat_dist_sq(m->pos, target) > lose * lose;
}

bool voxl_mob_chase_in_reach(const voxl_mob *m, vec3 target, float reach) {
    return voxl_mob_flat_dist_sq(m->pos, target) <= reach * reach;
}
