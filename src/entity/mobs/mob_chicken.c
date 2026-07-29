#include "mob_chicken.h"
#include "mob_passive.h"

// chickens flap: when off the ground we damp downward velocity so they
// flutter instead of plummet. caller still does the actual collision.
static void voxl_mob_chicken_flap(voxl_mob *m, float dt) {
    if (!m->on_ground && m->vel.y < 0.0f) {
        // clamp fall speed; feels like gliding.
        float terminal = -1.5f;
        if (m->vel.y < terminal) m->vel.y = terminal;
        // tiny upward nudge each flap.
        m->vel.y += 1.2f * dt;
    }
}

void voxl_mob_chicken_update(voxl_mob *m, voxl_mob_rng *r,
                             vec3 player_pos, float dt) {
    voxl_mob_passive_update(m, VOXL_MOB_CHICKEN, r, player_pos, dt);
    voxl_mob_chicken_flap(m, dt);
}

int voxl_mob_chicken_lay_egg(voxl_mob *m, voxl_mob_rng *r, float dt) {
    // attack_cd is unused for chickens, so we borrow it as the egg timer.
    if (m->attack_cd <= 0.0f) {
        // schedule the next egg somewhere in a wide window.
        m->attack_cd = voxl_mob_rng_frange(r, 30.0f, 60.0f);
        return 1;
    }
    m->attack_cd -= dt;
    return 0;
}
