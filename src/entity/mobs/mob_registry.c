#include "mob_registry.h"
#include "mob_stats.h"
#include "mob_health.h"

#include "mob_zombie.h"
#include "mob_skeleton.h"
#include "mob_creeper.h"
#include "mob_spider.h"
#include "mob_cow.h"
#include "mob_pig.h"
#include "mob_chicken.h"
#include "mob_sheep.h"

#include <string.h>

void voxl_mob_registry_init(voxl_mob_registry *reg, uint32_t seed) {
    memset(reg, 0, sizeof *reg);
    reg->next_id = 1;   // 0 is reserved for "no mob"
    voxl_mob_rng_seed(&reg->rng, seed);
}

uint32_t voxl_mob_registry_spawn(voxl_mob_registry *reg,
                                 voxl_mob_kind kind, vec3 pos) {
    for (int i = 0; i < VOXL_MOB_MAX; i++) {
        if (!reg->used[i]) {
            uint32_t id = reg->next_id++;
            voxl_mob_init(&reg->mobs[i], kind, pos, id);
            reg->used[i] = 1;
            reg->count++;
            return id;
        }
    }
    return 0;   // pool full
}

voxl_mob *voxl_mob_registry_get(voxl_mob_registry *reg, uint32_t id) {
    if (id == 0) return NULL;
    for (int i = 0; i < VOXL_MOB_MAX; i++) {
        if (reg->used[i] && reg->mobs[i].id == id) {
            return &reg->mobs[i];
        }
    }
    return NULL;
}

int voxl_mob_registry_remove(voxl_mob_registry *reg, uint32_t id) {
    for (int i = 0; i < VOXL_MOB_MAX; i++) {
        if (reg->used[i] && reg->mobs[i].id == id) {
            reg->used[i] = 0;
            reg->count--;
            return 1;
        }
    }
    return 0;
}

// route a single mob to the right per-kind brain. we throw away the melee
// damage / projectile flags here; a real game loop would forward them, but
// the registry just keeps the ai ticking.
static void voxl_mob_dispatch(voxl_mob *m, voxl_mob_rng *r,
                              vec3 player_pos, int is_dark, float dt) {
    int sink_fire = 0;
    switch (m->kind) {
    case VOXL_MOB_ZOMBIE:
        (void)voxl_mob_zombie_update(m, r, player_pos, dt);
        break;
    case VOXL_MOB_SKELETON:
        (void)voxl_mob_skeleton_update(m, r, player_pos, &sink_fire, dt);
        break;
    case VOXL_MOB_CREEPER:
        (void)voxl_mob_creeper_update(m, r, player_pos, dt);
        break;
    case VOXL_MOB_SPIDER:
        (void)voxl_mob_spider_update(m, r, player_pos, is_dark, dt);
        break;
    case VOXL_MOB_COW:
        voxl_mob_cow_update(m, r, player_pos, dt);
        break;
    case VOXL_MOB_PIG:
        voxl_mob_pig_update(m, r, player_pos, 0, dt);
        break;
    case VOXL_MOB_CHICKEN:
        voxl_mob_chicken_update(m, r, player_pos, dt);
        break;
    case VOXL_MOB_SHEEP:
        voxl_mob_sheep_update(m, r, player_pos, 0, dt);
        break;
    default:
        break;
    }
}

void voxl_mob_registry_update(voxl_mob_registry *reg,
                              vec3 player_pos, int is_dark, float dt) {
    for (int i = 0; i < VOXL_MOB_MAX; i++) {
        if (!reg->used[i]) continue;
        voxl_mob *m = &reg->mobs[i];

        voxl_mob_dispatch(m, &reg->rng, player_pos, is_dark, dt);

        // reap the dead once they've finished their hurt flash.
        if (voxl_mob_is_dead(m) && m->hurt_timer <= 0.0f) {
            reg->used[i] = 0;
            reg->count--;
        }
    }
}

int voxl_mob_registry_count(const voxl_mob_registry *reg) {
    return reg->count;
}
