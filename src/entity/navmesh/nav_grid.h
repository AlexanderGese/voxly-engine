#ifndef ENTITY_NAVMESH_NAV_GRID_H
#define ENTITY_NAVMESH_NAV_GRID_H
#include "nav_types.h"
#include "../../util/hashmap.h"
typedef struct {
    nav_cell *cells;        // flat array, owned. cap = NAV_MAX_CELLS.
    int       count;
    hashmap   index;        // nav_coord_key -> cell idx + 1
    int       full;         // hit the cap during build; mesh is partial
} nav_grid;
void nav_grid_init(nav_grid *g);
#endif
