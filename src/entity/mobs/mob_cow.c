#include "mob_cow.h"
#include "mob_passive.h"

// reuse attack_cd as the milk cooldown timer (cows never attack).

void voxl_mob_cow_update(voxl_mob *m, voxl_mob_rng *r, vec3 player_pos, float dt) {
    if (m->attack_cd > 0.0f) m->attack_cd -= dt;
    voxl_mob_passive_update(m, VOXL_MOB_COW, r, player_pos, dt);
}

int voxl_mob_cow_can_milk(const voxl_mob *m) {
    return m->attack_cd <= 0.0f;
}

void voxl_mob_cow_milked(voxl_mob *m) {
    m->attack_cd = 30.0f;   // half a minute between milkings
}
