#ifndef PLAYER_INVENTORY_VALIDATE_H
#define PLAYER_INVENTORY_VALIDATE_H

#include "inv_types.h"
#include "inv_grid.h"

// invariant checks + repair for a grid. nothing should ever produce a broken
// slot, but save files get hand-edited, builds change item max_stacks, and bugs
// happen — so on load (and in debug builds, every so often) we audit the grid
// and quietly fix what we can. better a clamped stack than a crash three frames
// later in the renderer.
//
// the rules a slot must obey:
// - id == NONE  <=>  count == 0           (no orphan counts, no ghost ids)
// - count <= item's max_stack             (no overfull stacks)
// - id < registry count                   (no dangling item ids)

typedef enum {
    INV_VERR_NONE = 0,
    INV_VERR_GHOST_ID,    // id set but count 0
    INV_VERR_ORPHAN_COUNT,// count set but id NONE
    INV_VERR_OVERFULL,    // count above the item's max_stack
    INV_VERR_BAD_ID,      // id not in the registry
} inv_verr;

// check a single slot without touching it. returns the first problem found.
inv_verr inv_validate_slot(const inv_stack *s);

// audit the whole grid read-only. returns the count of bad slots; if `first` is
// non-NULL it gets the index of the first offender (-1 if clean).
int inv_validate_grid(const inv_grid *g, int *first);

// repair in place: clears ghosts/orphans to proper empties, clamps overfull
// stacks to max, and empties slots holding unknown item ids. returns the number
// of slots it had to touch.
int inv_validate_repair(inv_grid *g);

// label for an error code, for logging.
const char *inv_validate_err_name(inv_verr e);

#endif
