#include "projectile_adapter.h"

#include <stddef.h>

// sampler shim: hand world_get_block straight through. user is the world*.
static block_id adapter_block_at(void *user, int wx, int wy, int wz) {
    world *w = (world *)user;
    // out-of-range y is air up top, solid-ish nothing at the very bottom; the
    // void check in the core handles the falling-out case so air is fine here.
    if (wy < 0 || wy >= CHUNK_SIZE_Y) return BLOCK_AIR;
    return world_get_block(w, wx, wy, wz);
}

// damage shim: look the mob up and dock hp + set the hurt flash. we deliberately
// dont kill here; the mob update / combat code owns death so loot + xp fire in
// one place. we just apply the number.
static void adapter_on_damage(void *user, int entity_id, int dmg, vec3 point) {
    projectile_adapter *pa = (projectile_adapter *)user;
    (void)point;
    if (!pa->mr || entity_id < 0) return;
    entity *e = mob_by_id(pa->mr, (uint32_t)entity_id);
    if (!e || !e->alive) return;

    e->hp -= dmg;
    e->hurt_timer = 0.4f;       // matches the flash combat.c uses
    if (e->hp <= 0) {
        e->hp = 0;
        e->alive = 0;           // mob_update_all will sweep the corpse
    }
}

void projectile_adapter_init(projectile_adapter *pa, world *w, uint64_t seed) {
    projectile_world_init(&pa->pw, seed);
    pa->w = w;
    pa->mr = NULL;
    pa->target_count = 0;

    projectile_sampler s = { adapter_block_at, w };
    projectile_world_set_sampler(&pa->pw, s);
    projectile_world_set_damage_cb(&pa->pw, adapter_on_damage, pa);
}

uint32_t projectile_adapter_fire(projectile_adapter *pa,
                                 const projectile_shot *shot) {
    return projectile_world_fire(&pa->pw, shot);
}

// build the frame's target list from live mobs, skipping dead ones. we use the
// mob's own aabb so the boxes line up with what the player sees and what melee
// combat uses — no second source of truth for hitboxes.
static void rebuild_targets(projectile_adapter *pa, mob_registry *mr) {
    int n = 0;
    for (int i = 0; i < mr->count && n < MAX_MOBS; i++) {
        entity *e = &mr->list[i];
        if (!e->alive || e->id == 0) continue;
        pa->targets[n].id  = (int)e->id;
        pa->targets[n].box = entity_aabb(e);
        n++;
    }
    pa->target_count = n;
}

void projectile_adapter_update(projectile_adapter *pa, mob_registry *mr,
                               float dt) {
    pa->mr = mr;                // bound so the damage cb can reach it
    rebuild_targets(pa, mr);

    projectile_world_update(&pa->pw, dt, pa->targets, pa->target_count);

    pa->mr = NULL;              // dont leave a dangling registry across frames
}
