#include "mob_skeleton.h"
#include "mob_util.h"
#include "mob_stats.h"
#include "mob_health.h"
#include "behavior_wander.h"
#include "behavior_chase.h"
#include "behavior_flee.h"

// skeleton likes to stay roughly this far from the player.
#define VOXL_SKELE_PREFERRED  6.0f
#define VOXL_SKELE_BAND       1.5f

int voxl_mob_skeleton_update(voxl_mob *m, voxl_mob_rng *r,
                             vec3 player_pos, int *out_fire, float dt) {
    const voxl_mob_spec *s = voxl_mob_spec_get(VOXL_MOB_SKELETON);
    if (out_fire) *out_fire = 0;
    int damage = 0;

    voxl_mob_health_tick(m, dt);
    if (voxl_mob_is_dead(m)) return 0;
    if (m->attack_cd > 0.0f) m->attack_cd -= dt;

    float dist = voxl_mob_flat_dist(m->pos, player_pos);

    switch (m->state) {
    case VOXL_MS_IDLE:
    case VOXL_MS_WANDER:
        voxl_mob_wander_tick(m, r, s->walk_speed, 8.0f, dt);
        if (dist < s->sight_range) {
            m->state = VOXL_MS_CHASE;
            m->has_wander_target = 0;
        }
        break;

    case VOXL_MS_CHASE:
        // always face the player so we can shoot.
        m->yaw = voxl_mob_yaw_to(m->pos, player_pos);

        if (dist > VOXL_SKELE_PREFERRED + VOXL_SKELE_BAND) {
            // too far, close in.
            voxl_mob_chase_tick(m, player_pos, s->run_speed, dt);
        } else if (dist < VOXL_SKELE_PREFERRED - VOXL_SKELE_BAND) {
            // too close, back off (reuse flee steering).
            voxl_mob_flee_tick(m, player_pos, s->run_speed, dt);
            m->yaw = voxl_mob_yaw_to(m->pos, player_pos);
        }
        // else: in the band, hold position and fire.

        if (dist <= s->attack_reach && m->attack_cd <= 0.0f) {
            damage = s->attack_damage;
            if (out_fire) *out_fire = 1;
            m->attack_cd = voxl_mob_rng_frange(r, 1.2f, 2.2f);
        }

        if (voxl_mob_chase_should_give_up(m, player_pos,
                                          s->sight_range + 8.0f)) {
            m->state = VOXL_MS_WANDER;
        }
        break;

    default:
        m->state = VOXL_MS_WANDER;
        break;
    }

    return damage;
}
