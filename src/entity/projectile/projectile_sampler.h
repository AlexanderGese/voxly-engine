#ifndef ENTITY_PROJECTILE_SAMPLER_H
#define ENTITY_PROJECTILE_SAMPLER_H

#include "../../world/block.h"

// the collision code never touches a world directly. instead it asks a sampler
// "what block is at this voxel?" through a function pointer. the real game wires
// this to world_get_block; the test harness wires it to a flat array. this is
// the seam that lets the ballistics live in its own little box.

typedef block_id (*projectile_block_at_fn)(void *user, int wx, int wy, int wz);

typedef struct {
    projectile_block_at_fn block_at;
    void *user;
} projectile_sampler;

// convenience: query, with a NULL-sampler guard returning air. callers lean on
// this so they dont have to null-check the function pointer everywhere.
static inline block_id projectile_sample(const projectile_sampler *s,
                                         int wx, int wy, int wz) {
    if (!s || !s->block_at) return BLOCK_AIR;
    return s->block_at(s->user, wx, wy, wz);
}

// classify a sampled block for collision purposes. these mirror block.h's
// helpers but live here so the sampler seam stays self-contained and the test
// harness can answer them without linking the whole world module.
int projectile_block_is_solid(block_id id);
int projectile_block_is_fluid(block_id id);

#endif
