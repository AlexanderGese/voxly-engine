#ifndef ENTITY_EPHYSICS_BROADPHASE_H
#define ENTITY_EPHYSICS_BROADPHASE_H
#include "ephysics_types.h"
#include "../../world/world.h"
#define EPHYS_MAX_CANDIDATES 256
typedef struct {
    aabb     boxes[EPHYS_MAX_CANDIDATES];
    block_id ids[EPHYS_MAX_CANDIDATES];
    int      count;
    int      overflowed;   // hit the cap. rare, but log-worthy if it happens
} ephys_candidates;
#endif
