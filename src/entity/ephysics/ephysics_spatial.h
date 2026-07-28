#ifndef ENTITY_EPHYSICS_SPATIAL_H
#define ENTITY_EPHYSICS_SPATIAL_H

#include "ephysics_types.h"
#include "../entity.h"

// uniform-grid spatial hash for entity-vs-entity broadphase. the block sweep
// handles entity-vs-world; this answers "which other entities are near me" so
// crowding mobs can shove apart and projectiles can find what they hit without
// an O(n^2) scan. cells are square in the xz plane (mobs rarely stack on y, so
// a 2d hash is plenty and cheaper).
//
// it's a rebuild-every-tick structure: cheap to clear, you reinsert all live
// entities each step. no removal, no incremental updates, dont overthink it.

#define EPHYS_GRID_CELL    2.0f    // meters per cell. ~one mob footprint
#define EPHYS_GRID_BUCKETS 1024    // power of two, mask-indexed
#define EPHYS_GRID_DEPTH   8       // entities kept per bucket before we drop

typedef struct {
    int   ids[EPHYS_GRID_DEPTH];   // entity array indices, not entity->id
    int   count;
} ephys_grid_bucket;

typedef struct {
    ephys_grid_bucket buckets[EPHYS_GRID_BUCKETS];
    int dropped;                   // inserts lost to full buckets, for debug
} ephys_grid;

// clear all buckets. call at the top of each tick before reinserting.
void ephysics_grid_clear(ephys_grid *g);

// insert entity index `idx` at world position `pos`. silently drops if the
// bucket is full (bumps `dropped`).
void ephysics_grid_insert(ephys_grid *g, int idx, vec3 pos);

// build the grid from a whole entity array in one call.
void ephysics_grid_build(ephys_grid *g, const entity *ents, int n);

// gather entity indices within `radius` meters (xz) of `pos` into `out`. returns
// how many were written, never more than `cap`. may include `pos`'s own entity;
// the caller filters by index.
int ephysics_grid_query(const ephys_grid *g, vec3 pos, float radius,
                        int *out, int cap);

#endif
