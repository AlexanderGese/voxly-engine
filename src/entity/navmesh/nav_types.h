#ifndef ENTITY_NAVMESH_NAV_TYPES_H
#define ENTITY_NAVMESH_NAV_TYPES_H
#include "../../math/vec3.h"
#include <stdint.h>
// shared types for the navmesh. unlike the pf_* pathfinder next door, which
// re-scans the world every search, this thing is baked once per region of
// chunks: we walk the walkable surface, drop a cell on every floor, link
// adjacent cells, then flood the link graph into connected regions. mobs
// then plan over cells instead of raw blocks, which is a lot cheaper when
// the world isn't changing under them.
//
// everything lives in block space (ints). vec3 conversion is at the edges.
// area we bake at once, in blocks. a 3x3 chunk patch is 48x48. we keep a one
// chunk apron so links near the edge still see their neighbour's floor.
#define NAV_PATCH_CHUNKS   3
#define NAV_PATCH_W        (NAV_PATCH_CHUNKS * 16)
#define NAV_APRON          1
#define NAV_SCAN_W         (NAV_PATCH_W + NAV_APRON * 2)
// vertical agent shape. one floor + this many clear blocks above = a cell.
// matches the mobs we actually spawn (2 tall).
#define NAV_AGENT_HEIGHT   2
#define NAV_STEP_UP        1
#define NAV_STEP_DOWN      3
#define NAV_JUMP_DOWN      4    // a jump-down link tolerates a bigger fall
#define NAV_MAX_CELLS      8192
#define NAV_CELL_LINKS     8
#define NAV_REGION_NONE    0
enum {
    NAV_LINK_WALK = 0,   // same height-ish, just step across
    NAV_LINK_DROP,       // floor below, controlled fall
    NAV_LINK_JUMP,       // floor above or a gap, needs a hop
}
;
typedef struct {
    int16_t x, y, z;     // block coord of the floor the feet rest on
    uint16_t region;     // connected-component id, 0 until flooded
    uint8_t  link_count;
    uint8_t  _pad;
    // outgoing links. `to` indexes into the cell array. parallel arrays keep
    // the struct tight; we touch link_count first in the hot loops anyway.
    int32_t  link_to  [NAV_CELL_LINKS];
    uint8_t  link_kind[NAV_CELL_LINKS];
    uint8_t  link_cost[NAV_CELL_LINKS];
} nav_cell;
typedef struct {
    int16_t x, y, z;
} nav_coord;
static inline nav_coord nav_coord_make(int x, int y, int z) {
    nav_coord c = { (int16_t)x, (int16_t)y, (int16_t)z };
    return c;
}

static inline int nav_coord_eq(nav_coord a, nav_coord b) {
    return a.x == b.x && a.y == b.y && a.z == b.z;
}

// pack a coord into a u64 for the hashmap. y is shallow so 16 bits is fine.
static inline uint64_t nav_coord_key(nav_coord c) {
    uint64_t x = (uint64_t)(uint16_t)c.x;
    uint64_t y = (uint64_t)(uint16_t)c.y;
    uint64_t z = (uint64_t)(uint16_t)c.z;
    return x | (y << 16) | (z << 32);
}

static inline nav_coord nav_cell_coord(const nav_cell *c) {
    return nav_coord_make(c->x, c->y, c->z);
}

// world-space centre of the block an agent's feet rest on. half-block offsets
// put it in the middle of the tile, standing on top of the floor block.
static inline vec3 nav_cell_world(const nav_cell *c) {
    return vec3_new(c->x + 0.5f, (float)(c->y + 1), c->z + 0.5f);
}
#endif
