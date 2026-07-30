#ifndef ENTITY_NAVMESH_NAV_DIRTY_H
#define ENTITY_NAVMESH_NAV_DIRTY_H

#include "nav_grid.h"
#include "../../world/world.h"

// incremental relink. a full patch bake isn't cheap, and most edits touch one
// block: a mob digs a step, a tree is felled, a door opens. rather than throw
// the whole grid out (which navmesh.c does on a coarse invalidate) we can patch
// the links around the edited column and re-flood, which is much cheaper for a
// localised change.
//
// caveat: this fixes *links*, not cell membership. if an edit creates a brand
// new floor where there wasn't one, that needs a real rebake — we detect that
// case and report it so the caller can fall back. so this is an optimisation
// for "the floors didn't move, only the connectivity did" edits, which is most
// of them.

// blast radius around an edited column, in blocks, whose cells we relink. one
// block each way covers the cardinal neighbours that could have gained or lost
// a link to the edited spot.
#define NAV_DIRTY_RADIUS  1

// queue of edited columns waiting for a relink pass. small fixed ring; if it
// overflows we just set `overflow` and the caller should do a full rebake.
#define NAV_DIRTY_CAP  64

typedef struct {
    int  x[NAV_DIRTY_CAP];
    int  z[NAV_DIRTY_CAP];
    int  count;
    int  overflow;     // 1 if we dropped edits; full rebake recommended
    int  needs_reflood;// 1 if any relink changed the graph topology
} nav_dirty;

void nav_dirty_init(nav_dirty *d);
void nav_dirty_clear(nav_dirty *d);

// note that world block column (wx, wz) changed. dedupes against the pending
// queue so spamming the same column is free. sets `overflow` if the queue is
// full.
void nav_dirty_mark(nav_dirty *d, int wx, int wz);

// relink every cell within NAV_DIRTY_RADIUS of each queued column against the
// current world, then re-flood regions if anything moved. clears the queue.
// returns 1 if the grid was patched in place, or 0 if a full rebake is needed
// (queue overflowed, or an edit implies new/removed cells we can't patch).
int nav_dirty_apply(nav_dirty *d, nav_grid *g, world *w);

#endif
