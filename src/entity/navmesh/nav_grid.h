#ifndef ENTITY_NAVMESH_NAV_GRID_H
#define ENTITY_NAVMESH_NAV_GRID_H
#include "nav_types.h"
#include "../../util/hashmap.h"
// the cell store + spatial index. cells live in one flat array (so links can
// be plain int32 indices and the whole thing serialises trivially); the
// hashmap maps a floor coord -> (cell index + 1) so linking and queries find
// a cell in O(1). +1 because the map can't tell 0 from absent.
typedef struct {
    nav_cell *cells;        // flat array, owned. cap = NAV_MAX_CELLS.
    int       count;
    hashmap   index;        // nav_coord_key -> cell idx + 1
    int       full;         // hit the cap during build; mesh is partial
} nav_grid;
void nav_grid_init(nav_grid *g);
void nav_grid_free(nav_grid *g);
void nav_grid_reset(nav_grid *g);
// reuse: clears cells + map, keeps memory
// add a cell for a floor coord. returns its index, or -1 if the pool is full
// (sets g->full). does nothing and returns the existing index if the coord is
// already present, so callers can be sloppy about duplicates.
int  nav_grid_add(nav_grid *g, int x, int y, int z);
// find the cell index registered at exactly this floor coord, or -1.
int  nav_grid_find(const nav_grid *g, nav_coord c);
static inline nav_cell *nav_grid_at(nav_grid *g, int idx) {
    return &g->cells[idx];
}
#endif
