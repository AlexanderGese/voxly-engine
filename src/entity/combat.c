#include "combat.h"
#include "../math/aabb.h"

#include <math.h>

void combat_player_attack(mob_registry *mr, const player *p, int reach_blocks) {
    vec3 forward = {sinf(p->yaw), 0, -cosf(p->yaw)};
    vec3 tip = vec3_add(p->pos, vec3_scale(forward, (float)reach_blocks));
    for (int i = 0; i < MAX_MOBS; i++) {
        entity *e = &mr->list[i];
        if (!e->alive) continue;
        if (vec3_distance(e->pos, tip) < 1.2f) {
            e->hp -= 4;
            e->hurt_timer = 0.25f;
            break;
        }
    }
}

void combat_mobs_hit_player(mob_registry *mr, const player *p, damage *d) {
    for (int i = 0; i < MAX_MOBS; i++) {
        entity *e = &mr->list[i];
        if (!e->alive) continue;
        if (e->ai_state != 3 /* AI_ATTACK */) continue;
        if (vec3_distance(e->pos, p->pos) < 1.6f) {
            damage_hurt(d, 2);
        }
    }
}
