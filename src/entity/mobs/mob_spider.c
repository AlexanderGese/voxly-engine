#include "mob_spider.h"
#include "mob_util.h"
#include "mob_stats.h"
#include "mob_health.h"
#include "behavior_wander.h"
#include "behavior_chase.h"

int voxl_mob_spider_update(voxl_mob *m, voxl_mob_rng *r,
                           vec3 player_pos, int is_dark, float dt) {
    const voxl_mob_spec *s = voxl_mob_spec_get(VOXL_MOB_SPIDER);
    int damage = 0;

    voxl_mob_health_tick(m, dt);
    if (voxl_mob_is_dead(m)) return 0;
    if (m->attack_cd > 0.0f) m->attack_cd -= dt;

    float dist = voxl_mob_flat_dist(m->pos, player_pos);

    switch (m->state) {
    case VOXL_MS_IDLE:
    case VOXL_MS_WANDER:
        voxl_mob_wander_tick(m, r, s->walk_speed, 8.0f, dt);
        // only turns hostile in the dark, and only once it sees you.
        if (is_dark && dist < s->sight_range) {
            m->state = VOXL_MS_CHASE;
            m->has_wander_target = 0;
        }
        break;

    case VOXL_MS_CHASE: {
        // spiders pounce: alternate short bursts of speed with pauses.
        // state_timer counts the burst phase.
        m->state_timer -= dt;
        float speed = s->run_speed;
        if (m->state_timer <= 0.0f) {
            // pick a new burst/pause cycle.
            m->state_timer = voxl_mob_rng_frange(r, 0.4f, 1.1f);
        }
        // jitter the heading a touch so the chase looks skittery.
        float want = voxl_mob_yaw_to(m->pos, player_pos);
        want += voxl_mob_rng_frange(r, -0.25f, 0.25f);
        m->yaw = voxl_mob_turn_toward(m->yaw, want, 8.0f * dt);
        voxl_mob_walk_forward(m, speed, dt);

        if (voxl_mob_chase_in_reach(m, player_pos, s->attack_reach)) {
            if (m->attack_cd <= 0.0f) {
                damage = s->attack_damage;
                m->attack_cd = 0.7f;
            }
        }
        // calm spiders return to wandering: give up if far, or if it got light.
        if (!is_dark ||
            voxl_mob_chase_should_give_up(m, player_pos, s->sight_range + 6.0f)) {
            m->state = VOXL_MS_WANDER;
        }
        break;
    }

    default:
        m->state = VOXL_MS_WANDER;
        break;
    }

    return damage;
}
