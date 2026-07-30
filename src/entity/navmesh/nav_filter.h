#ifndef ENTITY_NAVMESH_NAV_FILTER_H
#define ENTITY_NAVMESH_NAV_FILTER_H

#include "nav_grid.h"
#include <stdint.h>

// agent-aware traversal filter. the baked grid is one shape but the mobs that
// walk it are not: a spider happily takes jump links a cow never could, and a
// skittish mob might refuse a 4-block drop the planner would otherwise love.
// rather than bake a separate mesh per mob we bake one and mask the links at
// query/search time through one of these.
//
// the filter is tiny and copyable; movers keep one on their ai state and pass
// it down into nav_path / nav_cost. nothing here mutates the grid.

// which link flavours an agent is willing to use. bit per NAV_LINK_* kind.
enum {
    NAV_ALLOW_WALK = 1u << NAV_LINK_WALK,
    NAV_ALLOW_DROP = 1u << NAV_LINK_DROP,
    NAV_ALLOW_JUMP = 1u << NAV_LINK_JUMP,
    NAV_ALLOW_ALL  = NAV_ALLOW_WALK | NAV_ALLOW_DROP | NAV_ALLOW_JUMP,
};

typedef struct {
    uint8_t kinds;        // bitmask of NAV_ALLOW_*
    uint8_t max_cost;     // reject any single link costlier than this. 0 = no cap.
    uint8_t avoid_region; // a region id to steer clear of, or NAV_REGION_NONE.
    uint8_t _pad;
} nav_filter;

// the permissive default: everything goes, no cost cap. good for the planner
// when you don't care, and the base the per-mob presets tweak from.
nav_filter nav_filter_default(void);

// presets keyed off the entity_type enum next door. we don't include entity.h
// here to keep the dependency one-way; the caller passes the raw int type.
// unknown types fall back to the default.
nav_filter nav_filter_for_type(int entity_type);

// may an agent with this filter traverse link index `li` out of cell `ci`?
// checks the kind mask and the per-link cost cap. cheap, inlined in the hot
// search loop.
int nav_filter_allows(const nav_filter *f, const nav_grid *g, int ci, int li);

// would this filter even let the agent stand on cell `ci`? only the avoid
// region applies at the node level; everything else is per-link.
int nav_filter_cell_ok(const nav_filter *f, const nav_grid *g, int ci);

#endif
