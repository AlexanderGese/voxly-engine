#ifndef ENTITY_NAVMESH_NAV_TYPES_H
#define ENTITY_NAVMESH_NAV_TYPES_H
#include "../../math/vec3.h"
#include <stdint.h>
#define NAV_PATCH_CHUNKS   3
#define NAV_PATCH_W        (NAV_PATCH_CHUNKS * 16)
#define NAV_APRON          1
#define NAV_SCAN_W         (NAV_PATCH_W + NAV_APRON * 2)
#define NAV_AGENT_HEIGHT   2
#define NAV_STEP_UP        1
#define NAV_STEP_DOWN      3
#define NAV_JUMP_DOWN      4    // a jump-down link tolerates a bigger fall
#define NAV_MAX_CELLS      8192
#define NAV_CELL_LINKS     8
#define NAV_REGION_NONE    0
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
#endif
