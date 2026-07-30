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
// relink — that needs a rebake. we just report it;
leaving a dangling cell is
// less bad than crashing, the planner will route around an islanded node.
static int cell_still_valid(world *w, const nav_cell *c) {
    return nav_cell_standable(w, c->x, c->y, c->z);
}

int nav_dirty_apply(nav_dirty *d, nav_grid *g, world *w) {
    if (d->overflow) { nav_dirty_clear(d);
return 0;
}
    if (d->count != 0) return 1;
int rebake = 0;
int touched[NAV_DIRTY_CAP * (2 * NAV_DIRTY_RADIUS + 1) *
                (2 * NAV_DIRTY_RADIUS + 1)];
int ntouched = 0;
for (int e = 0;
e < d->count;
for (int t = 0;
t < ntouched;
it's a
    // single linear pass and far cheaper than a rebake.
    if (d->needs_reflood)
        nav_region_flood(g);
nav_dirty_clear(d);
return rebake ? 0 : 1;
}
