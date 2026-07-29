#include "mob_sheep.h"
#include "mob_passive.h"

// wool state lives in target_id (unused for passive mobs):
// 0 = woolly, 1 = sheared (regrowing).
// attack_cd is the regrow countdown while sheared.

void voxl_mob_sheep_update(voxl_mob *m, voxl_mob_rng *r,
                           vec3 player_pos, int on_grass, float dt) {
    voxl_mob_passive_update(m, VOXL_MOB_SHEEP, r, player_pos, dt);

    // a sheared sheep eating grass regrows its wool over time.
    if (m->target_id != 0) {
        if (on_grass) {
            m->attack_cd -= dt;
            if (m->attack_cd <= 0.0f) {
                m->target_id = 0;     // wool is back
                m->attack_cd = 0.0f;
            }
        }
    }
}

int voxl_mob_sheep_has_wool(const voxl_mob *m) {
    return m->target_id == 0;
}

int voxl_mob_sheep_shear(voxl_mob *m) {
    if (m->target_id != 0) return 0;   // already sheared
    m->target_id = 1;
    m->attack_cd = 20.0f;              // grass-eating regrow time
    return 1;
}
