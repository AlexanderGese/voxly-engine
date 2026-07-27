#ifndef ENTITY_ECS_SPATIAL_H
#define ENTITY_ECS_SPATIAL_H

#include "ecs_world.h"
#include "../../math/vec3.h"
#include "../../math/aabb.h"
#include "../../util/hashmap.h"

// uniform spatial hash over entity positions. the ai and collision passes both
// want "what's near me" and brute-forcing every pair is O(n^2) -- fine at 30
// mobs, ugly the moment a spawner goes wild. this buckets entities into a coarse
// grid keyed by floored cell coords and answers radius / aabb queries by only
// touching the overlapping cells.
//
// cell size is tied to CHUNK_SIZE_X so a query radius near a block or two only
// spans a couple cells. it's an xz grid -- mobs spread out horizontally far more
// than vertically in a voxel world, so paying for a third axis of cells buys
// almost nothing. the y coordinate just rides along in the stored record.

#define ECS_GRID_CELL    8      // blocks per cell edge (half a chunk)
#define ECS_GRID_QUERY_MAX 256  // cap on results from one query, keeps it bounded

typedef struct ecs_grid_item {
    ecs_entity            e;
    vec3                  pos;
    struct ecs_grid_item *next;   // intrusive chain within a cell bucket
} ecs_grid_item;

typedef struct {
    hashmap        cells;     // packed cell key -> ecs_grid_item* bucket head
    ecs_grid_item *pool;      // flat backing array of items, indexed by build order
    uint32_t       pool_len;
    uint32_t       pool_cap;
    uint32_t       count;     // entities inserted this build
} ecs_grid;

void ecs_grid_init(ecs_grid *g, uint32_t reserve);
void ecs_grid_free(ecs_grid *g);

// rebuild the grid from every entity with a transform. cheap enough to do every
// frame -- it's a single query pass plus a hashmap insert each.
void ecs_grid_build(ecs_grid *g, ecs_world *w);

// collect entities whose stored position is within `radius` of `center` (xz
// distance; y is compared too but with the same radius). writes up to out_cap
// handles, returns the count. excludes `ignore` so a mob doesnt find itself.
uint32_t ecs_grid_query_radius(const ecs_grid *g, vec3 center, float radius,
                               ecs_entity ignore, ecs_entity *out,
                               uint32_t out_cap);

// collect entities whose position falls inside `box`. same output contract.
uint32_t ecs_grid_query_aabb(const ecs_grid *g, aabb box, ecs_entity ignore,
                             ecs_entity *out, uint32_t out_cap);

// nearest other entity to `center`, or ECS_NULL if the grid is empty within the
// scanned cells. handy for "closest target" without sorting a full query.
ecs_entity ecs_grid_nearest(const ecs_grid *g, vec3 center, float max_radius,
                            ecs_entity ignore);

#endif
