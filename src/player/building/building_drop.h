#ifndef PLAYER_BUILDING_DROP_H
#define PLAYER_BUILDING_DROP_H

#include "building_break.h"
#include "../../math/vec3.h"
#include "../../math/rng.h"

// turns the abstract building_drop a break produces into concrete spawn orders
// the entity layer can chew on: a position, a little pop of velocity, and a
// stack size. we don't own the item-entity type (entity layer does), so this
// just hands back plain descriptors and lets the caller instantiate them.

typedef struct {
    block_id id;
    int      count;
    vec3     pos;     // world-space spawn point
    vec3     vel;     // initial velocity (the "pop")
} building_drop_spawn;

// a break can yield more than one stack (drop_count > stack split). this fans
// a building_drop into up to `max_out` spawn descriptors, splitting on the
// block's stack_max so a 70-count drop becomes 64 + 6. returns #written.
int building_drop_split(const building_drop *d, building_drop_spawn *out,
                        int max_out, rng *r);

// the single-stack case: one descriptor, centered with a small random pop.
building_drop_spawn building_drop_one(block_id id, int count, vec3 pos, rng *r);

// a tuned upward-biased random velocity so dropped items hop off the broken
// block instead of clipping into the floor. deterministic given the rng.
vec3 building_drop_pop_velocity(rng *r);

#endif
