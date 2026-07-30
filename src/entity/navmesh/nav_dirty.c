#include "nav_dirty.h"
#include "nav_link.h"
#include "nav_cell.h"
#include "nav_region.h"
#include "nav_build.h"   // NAV_SCAN_Y_MARGIN, the y band the bake uses

void nav_dirty_init(nav_dirty *d) {
    d->count = 0;
    d->overflow = 0;
    d->needs_reflood = 0;
}

void nav_dirty_clear(nav_dirty *d) {
    d->count = 0;
    d->overflow = 0;
    d->needs_reflood = 0;
}

void nav_dirty_mark(nav_dirty *d, int wx, int wz) {
    for (int i = 0; i < d->count; i++)
        if (d->x[i] == wx && d->z[i] == wz) return;   // already queued
    if (d->count >= NAV_DIRTY_CAP) { d->overflow = 1; return; }
    d->x[d->count] = wx;
    d->z[d->count] = wz;
    d->count++;
}

// wipe a single cell's outgoing link fan so it can be rebuilt from scratch.
// the incoming links from neighbours get rebuilt when those neighbours relink,
// which is why we relink the whole radius, not just the centre column.
static void clear_links(nav_cell *c) {
    c->link_count = 0;
}

// does the world still agree this cell is a valid floor? if an edit removed the
// floor block (or buried it), the cell is stale and we can't fix it with a
// relink — that needs a rebake. we just report it; leaving a dangling cell is
// less bad than crashing, the planner will route around an islanded node.
static int cell_still_valid(world *w, const nav_cell *c) {
    return nav_cell_standable(w, c->x, c->y, c->z);
}

int nav_dirty_apply(nav_dirty *d, nav_grid *g, world *w) {
    if (d->overflow) { nav_dirty_clear(d); return 0; }
    if (d->count == 0) return 1;   // nothing to do, trivially fine

    int rebake = 0;

    // pass 1: collect the cells in range and reset their link fans. we gather
    // indices up front because clearing one cell's links shouldn't perturb the
    // scan, and a cell can sit in two columns' radii.
    int touched[NAV_DIRTY_CAP * (2 * NAV_DIRTY_RADIUS + 1) *
                (2 * NAV_DIRTY_RADIUS + 1)];
    int ntouched = 0;

    for (int e = 0; e < d->count; e++) {
        for (int dz = -NAV_DIRTY_RADIUS; dz <= NAV_DIRTY_RADIUS; dz++) {
            for (int dx = -NAV_DIRTY_RADIUS; dx <= NAV_DIRTY_RADIUS; dx++) {
                int cx = d->x[e] + dx;
                int cz = d->z[e] + dz;
                // a column can hold several stacked cells (ledges, overhangs),
                // and we don't know their y up front. probe the index across
                // the shallow band the builder bakes; the grid is sparse so
                // most of these probes miss and cost nothing.
                for (int y = WORLD_SEA_LEVEL - NAV_SCAN_Y_MARGIN;
                     y < WORLD_SEA_LEVEL + NAV_SCAN_Y_MARGIN; y++) {
                    int idx = nav_grid_find(g, nav_coord_make(cx, y, cz));
                    if (idx < 0) continue;
                    nav_cell *c = nav_grid_at(g, idx);
                    if (!cell_still_valid(w, c)) { rebake = 1; continue; }
                    // dedupe the touched list; radii overlap.
                    int seen = 0;
                    for (int t = 0; t < ntouched; t++)
                        if (touched[t] == idx) { seen = 1; break; }
                    if (!seen) {
                        touched[ntouched++] = idx;
                        clear_links(c);
                    }
                }
            }
        }
    }

    // pass 2: relink every touched cell in all four cardinals. nav_link_step
    // also pushes the mirror link back onto the neighbour, so a touched cell's
    // neighbours get their inbound link restored even if they weren't touched.
    static const int dirs[4][2] = { {1,0}, {-1,0}, {0,1}, {0,-1} };
    for (int t = 0; t < ntouched; t++) {
        for (int dd = 0; dd < 4; dd++) {
            if (nav_link_step(g, w, touched[t], dirs[dd][0], dirs[dd][1]))
                d->needs_reflood = 1;
        }
    }

    // topology changed -> region labels may be stale. re-flood the lot; it's a
    // single linear pass and far cheaper than a rebake.
    if (d->needs_reflood)
        nav_region_flood(g);

    nav_dirty_clear(d);
    return rebake ? 0 : 1;
}
