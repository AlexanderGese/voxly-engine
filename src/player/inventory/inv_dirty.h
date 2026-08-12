#ifndef PLAYER_INVENTORY_DIRTY_H
#define PLAYER_INVENTORY_DIRTY_H

#include "inv_types.h"
#include "inv_grid.h"

// dirty-slot tracking for the hud. the inventory mutates a lot more often than
// the screen needs to redraw, so instead of re-uploading every slot quad each
// frame we mark touched slots and let the renderer pull just those.
//
// it's a bitset over a grid's slots plus a snapshot of the last-seen contents,
// so inv_dirty_scan() can diff after a batch of ops that didn't bother to mark
// anything (the save-load path, say). two ways in: mark explicitly, or scan.

typedef struct {
    uint32_t *bits;     // one bit per slot, owned
    inv_stack *shadow;  // last-known contents, owned, parallel to the grid
    int        count;   // slot count this tracker is sized for
    int        any;     // sticky "something is dirty" flag, cheap early-out
} inv_dirty;

void inv_dirty_init(inv_dirty *d, const inv_grid *g);
void inv_dirty_free(inv_dirty *d);

// mark one slot / a range / everything as needing redraw.
void inv_dirty_mark(inv_dirty *d, int idx);
void inv_dirty_mark_range(inv_dirty *d, int lo, int hi);
void inv_dirty_mark_all(inv_dirty *d);

int  inv_dirty_is_set(const inv_dirty *d, int idx);
int  inv_dirty_any(const inv_dirty *d);

// diff the grid against the shadow, marking every slot that changed and
// refreshing the shadow. returns how many slots flipped. use this when you
// didn't track edits as they happened.
int  inv_dirty_scan(inv_dirty *d, const inv_grid *g);

// clear all dirty bits (the renderer calls this once it has consumed them) and
// resync the shadow to the current grid so the next scan diffs from here.
void inv_dirty_consume(inv_dirty *d, const inv_grid *g);

// iterate the set bits. start with idx=-1; returns the next dirty slot index or
// -1 when done. doesn't clear anything.
int  inv_dirty_next(const inv_dirty *d, int idx);

#endif
