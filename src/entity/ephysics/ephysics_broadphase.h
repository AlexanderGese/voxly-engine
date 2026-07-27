#ifndef ENTITY_EPHYSICS_BROADPHASE_H
#define ENTITY_EPHYSICS_BROADPHASE_H
#include "ephysics_types.h"
#include "../../world/world.h"
// the broadphase turns "an entity wants to move from A to B" into a small list
// of solid block boxes that move could possibly hit. we gather over the swept
// aabb (start box unioned with end box) plus a one-block skin so step-up and
// the next tick's grounding check have data to work with.
#define EPHYS_MAX_CANDIDATES 256
typedef struct {
    aabb     boxes[EPHYS_MAX_CANDIDATES];
    block_id ids[EPHYS_MAX_CANDIDATES];
    int      count;
    int      overflowed;   // hit the cap. rare, but log-worthy if it happens
} ephys_candidates;
// fill `out` with solid block boxes overlapping the body's swept region for a
// move of `delta`. solidity comes from block_is_solid plus slab/plant handling.
void ephysics_gather(world *w, const ephys_body *b, vec3 delta,
                     ephys_candidates *out);
void ephysics_gather_fluid(world *w, const ephys_body *b, ephys_candidates *out);
int ephysics_block_box(world *w, int wx, int wy, int wz, block_id id, aabb *out);
#endif
