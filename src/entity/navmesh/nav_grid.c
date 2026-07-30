#include "nav_grid.h"

#include <stdlib.h>
#include <string.h>

void nav_grid_init(nav_grid *g) {
    g->cells = malloc(sizeof(nav_cell) * NAV_MAX_CELLS);
    g->count = 0;
    g->full  = 0;
    // a patch is ~48x48 with a layer or two, so a few thousand cells. seed
    // the map big enough to avoid a rehash storm during the build.
    hashmap_init(&g->index, 8192);
}

void nav_grid_free(nav_grid *g) {
    free(g->cells);
    g->cells = NULL;
    g->count = 0;
    hashmap_free(&g->index);
}

void nav_grid_reset(nav_grid *g) {
    g->count = 0;
    g->full  = 0;
    // hashmap has no clear-in-place, so bounce it. keeps the api honest.
    hashmap_free(&g->index);
    hashmap_init(&g->index, 8192);
}

int nav_grid_find(const nav_grid *g, nav_coord c) {
    void *v = hashmap_get(&g->index, nav_coord_key(c));
    if (!v) return -1;
    return (int)((intptr_t)v) - 1;
}

int nav_grid_add(nav_grid *g, int x, int y, int z) {
    nav_coord c = nav_coord_make(x, y, z);

    int existing = nav_grid_find(g, c);
    if (existing >= 0) return existing;

    if (g->count >= NAV_MAX_CELLS) {
        g->full = 1;
        return -1;
    }

    int idx = g->count++;
    nav_cell *cell = &g->cells[idx];
    memset(cell, 0, sizeof *cell);
    cell->x = (int16_t)x;
    cell->y = (int16_t)y;
    cell->z = (int16_t)z;
    cell->region = NAV_REGION_NONE;

    // store idx+1 so a real 0th cell isn't mistaken for "absent".
    hashmap_put(&g->index, nav_coord_key(c), (void*)(intptr_t)(idx + 1));
    return idx;
}
