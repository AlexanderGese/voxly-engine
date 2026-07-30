#include "mob_zombie.h"
#include "mob_util.h"
#include "mob_stats.h"
#include "mob_health.h"
#include "behavior_wander.h"
#include "behavior_chase.h"

int voxl_mob_zombie_update(voxl_mob *m, voxl_mob_rng *r,
                           vec3 player_pos, float dt) {
    const voxl_mob_spec *s = voxl_mob_spec_get(VOXL_MOB_ZOMBIE);
    int damage = 0;

    voxl_mob_health_tick(m, dt);
    if (voxl_mob_is_dead(m)) return 0;
    if (m->attack_cd > 0.0f) m->attack_cd -= dt;

    float dist = voxl_mob_flat_dist(m->pos, player_pos);

    // hurt flash interrupts whatever we were doing, briefly.
    if (m->hurt_timer > 0.0f && m->state != VOXL_MS_DEAD) {
        // stay angry: hurt zombies lunge at the player.
        m->state = VOXL_MS_CHASE;
    }

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
        if (voxl_mob_chase_in_reach(m, player_pos, s->attack_reach)) {
            m->state = VOXL_MS_ATTACK;
        } else if (voxl_mob_chase_should_give_up(m, player_pos,
                                                 s->sight_range + 8.0f)) {
            m->state = VOXL_MS_WANDER;
        }
        break;

    case VOXL_MS_ATTACK:
        // keep facing the player while winding up.
        m->yaw = voxl_mob_yaw_to(m->pos, player_pos);
        if (!voxl_mob_chase_in_reach(m, player_pos, s->attack_reach + 0.3f)) {
            m->state = VOXL_MS_CHASE;
        } else if (m->attack_cd <= 0.0f) {
            damage = s->attack_damage;
            m->attack_cd = 1.0f;   // one swing per second
        }
        break;

    default:
        m->state = VOXL_MS_WANDER;
        break;
    }

    return damage;
}
