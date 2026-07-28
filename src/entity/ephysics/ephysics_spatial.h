#ifndef ENTITY_EPHYSICS_SPATIAL_H
#define ENTITY_EPHYSICS_SPATIAL_H
#include "ephysics_types.h"
#include "../entity.h"
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
void ephysics_grid_clear(ephys_grid *g);
void ephysics_grid_insert(ephys_grid *g, int idx, vec3 pos);
void ephysics_grid_build(ephys_grid *g, const entity *ents, int n);
int ephysics_grid_query(const ephys_grid *g, vec3 pos, float radius,
                        int *out, int cap);
#endif
