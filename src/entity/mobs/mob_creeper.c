#include "mob_creeper.h"
#include "mob_util.h"
#include "mob_stats.h"
#include "mob_health.h"
#include "behavior_wander.h"
#include "behavior_chase.h"

#define VOXL_CREEPER_FUSE_TIME  1.5f   // seconds from ignite to boom
#define VOXL_CREEPER_TRIGGER    3.0f   // start hissing within this range
#define VOXL_CREEPER_BLAST      4.0f

// we stash the fuse charge in attack_cd (0..fuse_time). it is unused
// otherwise for the creeper, which never makes a melee swing.

voxl_mob_creeper_result
voxl_mob_creeper_update(voxl_mob *m, voxl_mob_rng *r,
                        vec3 player_pos, float dt) {
    const voxl_mob_spec *s = voxl_mob_spec_get(VOXL_MOB_CREEPER);
    voxl_mob_creeper_result res = { 0, 0.0f, 0.0f };

    voxl_mob_health_tick(m, dt);
    if (voxl_mob_is_dead(m)) return res;

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
        voxl_mob_chase_tick(m, player_pos, s->run_speed, dt);
        if (dist < VOXL_CREEPER_TRIGGER) {
            m->state = VOXL_MS_ATTACK;   // start the fuse
            m->attack_cd = 0.0f;
        } else if (voxl_mob_chase_should_give_up(m, player_pos,
                                                 s->sight_range + 8.0f)) {
            m->state = VOXL_MS_WANDER;
        }
        break;

    case VOXL_MS_ATTACK:
        // hold still and swell. fuse charges while the player is near,
        // and discharges (cancels) if they step away.
        if (dist < VOXL_CREEPER_TRIGGER + 0.5f) {
            m->attack_cd += dt;
        } else {
            m->attack_cd -= dt * 2.0f;   // cools off twice as fast
            if (m->attack_cd <= 0.0f) {
                m->attack_cd = 0.0f;
                m->state = VOXL_MS_CHASE;
            }
        }

        if (m->attack_cd >= VOXL_CREEPER_FUSE_TIME) {
            res.exploded = 1;
            res.blast_radius = VOXL_CREEPER_BLAST;
            m->health = 0;
            m->state = VOXL_MS_DEAD;
        }
        break;

    default:
        m->state = VOXL_MS_WANDER;
        break;
    }

    res.fuse = voxl_mob_clampf(m->attack_cd / VOXL_CREEPER_FUSE_TIME,
                               0.0f, 1.0f);
    return res;
}
