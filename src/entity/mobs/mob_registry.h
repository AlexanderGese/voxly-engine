#ifndef ENTITY_MOBS_MOB_REGISTRY_H
#define ENTITY_MOBS_MOB_REGISTRY_H

#include "mob_common.h"
#include "mob_rng.h"
#include "../../math/vec3.h"

// fixed-capacity pool of voxl_mobs plus the per-kind update dispatch.
// self contained: does not touch the engine's entity/mob systems.

#define VOXL_MOB_MAX 256

typedef struct {
    voxl_mob      mobs[VOXL_MOB_MAX];
    int           used[VOXL_MOB_MAX];   // slot occupied flag
    int           count;
    uint32_t      next_id;
    voxl_mob_rng  rng;
} voxl_mob_registry;

// set up an empty registry with a seeded prng.
void voxl_mob_registry_init(voxl_mob_registry *reg, uint32_t seed);

// spawn a mob of `kind` at `pos`. returns its id, or 0 if the pool is full.
uint32_t voxl_mob_registry_spawn(voxl_mob_registry *reg,
                                 voxl_mob_kind kind, vec3 pos);

// look up a live mob by id, or NULL.
voxl_mob *voxl_mob_registry_get(voxl_mob_registry *reg, uint32_t id);

// free a slot by id. returns 1 if it was found.
int voxl_mob_registry_remove(voxl_mob_registry *reg, uint32_t id);

// update every live mob against the player. `is_dark` drives night-only
// hostility (spiders). dead mobs are reaped automatically.
void voxl_mob_registry_update(voxl_mob_registry *reg,
                              vec3 player_pos, int is_dark, float dt);

// how many live mobs there are.
int voxl_mob_registry_count(const voxl_mob_registry *reg);

#endif
