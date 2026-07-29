#include "mob_pig.h"
#include "mob_passive.h"
#include "mob_util.h"
#include "mob_stats.h"
#include "mob_health.h"
#include "behavior_chase.h"

void voxl_mob_pig_update(voxl_mob *m, voxl_mob_rng *r,
                         vec3 player_pos, int has_carrot, float dt) {
    const voxl_mob_spec *s = voxl_mob_spec_get(VOXL_MOB_PIG);

    // carrot luring overrides normal passive behavior, unless we're fleeing.
    if (has_carrot && m->state != VOXL_MS_FLEE && m->hurt_timer <= 0.0f) {
        voxl_mob_health_tick(m, dt);
        if (voxl_mob_is_dead(m)) return;
        float d = voxl_mob_flat_dist(m->pos, player_pos);
        if (d > 2.0f && d < 10.0f) {
            // trot after the player, but not right on top of them.
            voxl_mob_chase_tick(m, player_pos, s->walk_speed, dt);
            return;
        }
        if (d <= 2.0f) {
            // close enough; just face them and wait.
            m->yaw = voxl_mob_yaw_to(m->pos, player_pos);
            return;
        }
    }

    voxl_mob_passive_update(m, VOXL_MOB_PIG, r, player_pos, dt);
}
