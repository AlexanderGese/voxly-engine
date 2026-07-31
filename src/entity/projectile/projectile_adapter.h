#ifndef ENTITY_PROJECTILE_ADAPTER_H
#define ENTITY_PROJECTILE_ADAPTER_H

#include "projectile_world.h"
#include "../mob.h"
#include "../../world/world.h"

// the glue that marries the engine-free projectile core to the actual game: it
// wires the block sampler to world_get_block, builds the per-frame target list
// from the mob registry, and routes damage callbacks into mob hp. everything
// game-specific is corralled here so the rest of the subsystem stays portable.

typedef struct {
    projectile_world pw;

    // bound for the duration of a frame's update so the damage callback can find
    // the mob registry without smuggling it through void* gymnastics.
    mob_registry *mr;
    world        *w;

    // scratch target buffer rebuilt each frame from live mobs.
    projectile_target targets[MAX_MOBS];
    int               target_count;
} projectile_adapter;

void projectile_adapter_init(projectile_adapter *pa, world *w, uint64_t seed);

// fire helper that takes the same shot params and forwards to the core.
uint32_t projectile_adapter_fire(projectile_adapter *pa,
                                 const projectile_shot *shot);

// run a frame: snapshot mob boxes, bind the sampler to this world, step the sim,
// and apply any damage to the registry. call once per game tick.
void projectile_adapter_update(projectile_adapter *pa, mob_registry *mr,
                               float dt);

#endif
