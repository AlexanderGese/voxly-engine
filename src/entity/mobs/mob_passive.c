#include "mob_passive.h"
#include "mob_util.h"
#include "mob_stats.h"
#include "mob_health.h"
#include "behavior_wander.h"
#include "behavior_flee.h"

#define VOXL_PASSIVE_SAFE_DIST  12.0f

void voxl_mob_passive_update(voxl_mob *m, voxl_mob_kind kind,
                             voxl_mob_rng *r, vec3 player_pos, float dt) {
    const voxl_mob_spec *s = voxl_mob_spec_get(kind);

    voxl_mob_health_tick(m, dt);
    if (voxl_mob_is_dead(m)) return;

    // a fresh hit (hurt timer near full) kicks the animal into flee mode.
    if (m->hurt_timer > 0.30f && m->state != VOXL_MS_FLEE) {
        m->state = VOXL_MS_FLEE;
        m->has_wander_target = 0;
        m->state_timer = 0.0f;
    }

    switch (m->state) {
    case VOXL_MS_FLEE:
        voxl_mob_flee_tick(m, player_pos, s->run_speed, dt);
        // run for at least a moment, then check if we're safe.
        m->state_timer += dt;
        if (m->state_timer > 1.0f &&
            voxl_mob_flee_is_safe(m, player_pos, VOXL_PASSIVE_SAFE_DIST)) {
            m->state = VOXL_MS_WANDER;
            m->has_wander_target = 0;
        }
        break;

    case VOXL_MS_IDLE:
    case VOXL_MS_WANDER:
    default:
        m->state = VOXL_MS_WANDER;
        voxl_mob_wander_tick(m, r, s->walk_speed, 6.0f, dt);
        break;
    }
}
