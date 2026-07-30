#include "nav_query.h"
#include "nav_region.h"

#include <math.h>
#include <stdlib.h>

// snap a float coord down to the block it sits in. floorf because negative
// coords have to round toward -inf, not toward zero.
static int blk(float v) { return (int)floorf(v); }

// look down a single column for a floor cell near feet height `fy`. checks fy
// first (standing exactly on it) then a couple below (just stepped off a
// ledge / mid-fall). returns cell idx or -1.
static int column_cell(const nav_grid *g, int x, int z, int fy) {
    // an entity's feet at world y rest on the block at y-1, so the floor cell
    // we want has cell->y == fy - 1. probe that, then a few lower.
    for (int dy = 0; dy <= NAV_STEP_DOWN + 1; dy++) {
        int idx = nav_grid_find(g, nav_coord_make(x, fy - 1 - dy, z));
        if (idx >= 0) return idx;
    }
    return -1;
}

int nav_query_cell_at(const nav_grid *g, vec3 p) {
    return column_cell(g, blk(p.x), blk(p.z), blk(p.y));
}

int nav_query_nearest(const nav_grid *g, vec3 p, int r) {
    int cx = blk(p.x), cz = blk(p.z), fy = blk(p.y);

    // exact column first, the common case.
    int hit = column_cell(g, cx, cz, fy);
    if (hit >= 0) return hit;

    // expanding square rings. stop at the first ring that yields anything;
    // ties within a ring resolve to whatever we hit first, good enough for
    // snapping a confused mob back onto the mesh.
    for (int ring = 1; ring <= r; ring++) {
        for (int dz = -ring; dz <= ring; dz++) {
            for (int dx = -ring; dx <= ring; dx++) {
                // only the perimeter of this ring; interior was done already.
                if (abs(dx) != ring && abs(dz) != ring) continue;
                int idx = column_cell(g, cx + dx, cz + dz, fy);
                if (idx >= 0) return idx;
            }
        }
    }
    return -1;
}

int nav_query_reachable(const nav_grid *g, int a, int b) {
    if (a < 0 || b < 0) return 0;
    if (a == b) return 1;
    return nav_region_connected(g, a, b);
}

vec3 nav_query_snap(const nav_grid *g, vec3 p) {
    int idx = nav_query_nearest(g, p, 2);
    if (idx < 0) return p;
    return nav_cell_world(&g->cells[idx]);
}
