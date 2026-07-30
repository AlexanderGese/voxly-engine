#include "nav_build.h"
#include "nav_cell.h"
#include "nav_link.h"
#include "nav_region.h"
#include "../../config.h"
// per-column floor scratch. a column rarely has more than a handful of floors
#define NAV_COL_FLOORS 16
int nav_build_patch(nav_grid *g, world *w, int pcx, int pcz,
                    nav_build_stats *stats) {
    nav_grid_reset(g);

    // world-space block bounds of the scan area: the patch plus the apron, so
    // edge cells can still find a neighbour floor to link to.
    int wx0 = pcx * CHUNK_SIZE_X - NAV_APRON;
    int wz0 = pcz * CHUNK_SIZE_Z - NAV_APRON;
    int wx1 = wx0 + NAV_SCAN_W;
    int wz1 = wz0 + NAV_SCAN_W;

    // vertical band. centre on sea level; that's where the walkable world is.
    int y_lo = WORLD_SEA_LEVEL - NAV_SCAN_Y_MARGIN;
    int y_hi = WORLD_SEA_LEVEL + NAV_SCAN_Y_MARGIN;
    if (y_lo < 1)               y_lo = 1;            // y0 has no block below it
    if (y_hi > CHUNK_SIZE_Y - 2) y_hi = CHUNK_SIZE_Y - 2;

    int16_t floors[NAV_COL_FLOORS];

    // pass 1: drop a cell on every floor span in every column.
    for (int wz = wz0; wz < wz1; wz++) {
        for (int wx = wx0; wx < wx1; wx++) {
            int nf = nav_cell_spans(w, wx, wz, y_lo, y_hi,
                                    floors, NAV_COL_FLOORS);
            for (int i = 0; i < nf; i++) {
                if (nav_grid_add(g, wx, floors[i], wz) < 0)
                    goto linked;   // pool full, stop placing
            }
        }
    }

linked:
    // pass 2: link cardinal neighbours. needs every cell already placed.
    nav_link_all(g, w);

    // pass 3: flood connected components.
    int regions = nav_region_flood(g);

    if (stats) {
        stats->pcx     = pcx;
        stats->pcz     = pcz;
        stats->cells   = g->count;
        stats->regions = regions;
        stats->partial = g->full;
    }
    return g->count;
}
