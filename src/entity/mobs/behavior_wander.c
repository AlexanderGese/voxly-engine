#include "behavior_wander.h"
#include "mob_util.h"

#include <math.h>

#ifndef VOXL_PI
#define VOXL_PI 3.14159265358979323846f
#endif

void voxl_mob_wander_pick(voxl_mob *m, voxl_mob_rng *r, float radius) {
    float ang = voxl_mob_rng_frange(r, -VOXL_PI, VOXL_PI);
    float dist = voxl_mob_rng_frange(r, 1.0f, radius);
    m->wander_target.x = m->pos.x + sinf(ang) * dist;
    m->wander_target.y = m->pos.y;
    m->wander_target.z = m->pos.z - cosf(ang) * dist;
    m->has_wander_target = 1;
    // a little dwell time so mobs don't repick every frame.
    m->state_timer = voxl_mob_rng_frange(r, 2.0f, 5.0f);
}

void voxl_mob_wander_tick(voxl_mob *m, voxl_mob_rng *r,
                          float speed, float radius, float dt) {
    m->state_timer -= dt;

    if (!m->has_wander_target || m->state_timer <= 0.0f) {
        voxl_mob_wander_pick(m, r, radius);
        return;
    }

    // close enough? stop and idle a beat before repicking.
    float d = voxl_mob_flat_dist(m->pos, m->wander_target);
    if (d < 0.6f) {
        m->has_wander_target = 0;
        m->state_timer = voxl_mob_rng_frange(r, 0.5f, 2.0f);
        return;
    }

    // steer: smoothly face the target then step forward.
    float want = voxl_mob_yaw_to(m->pos, m->wander_target);
    m->yaw = voxl_mob_turn_toward(m->yaw, want, 4.0f * dt);
    voxl_mob_walk_forward(m, speed, dt);
}
