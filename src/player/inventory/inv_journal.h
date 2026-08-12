#ifndef PLAYER_INVENTORY_JOURNAL_H
#define PLAYER_INVENTORY_JOURNAL_H

#include "inv_types.h"
#include "inv_grid.h"

// a tiny undo journal for inventory edits. every slot change is recorded as a
// (slot, before, after) triple; an undo walks them backwards and restores the
// `before` snapshots. this is what backs ctrl-z in the inventory screen and,
// more usefully, lets a botched quick-move-everything be rolled back in one go.
//
// the journal is a ring of fixed depth. when it fills, the oldest edits fall off
// the back — you can't undo forever, but you can undo the last few clicks, which
// is all anyone actually does.

#define INV_JOURNAL_DEPTH   64

typedef struct {
    int       slot;     // which grid slot changed
    inv_stack before;   // contents prior to the edit
    inv_stack after;    // contents after the edit
    uint32_t  group;    // edits sharing a group undo together (one click = one)
} inv_journal_entry;

typedef struct {
    inv_grid          *grid;    // borrowed, the grid we journal edits against
    inv_journal_entry  ring[INV_JOURNAL_DEPTH];
    int                head;    // next write index (mod DEPTH)
    int                len;     // valid entries, <= DEPTH
    uint32_t           group;   // current group id, bumped by begin()
    int                in_group;// are we inside a begin/end pair
} inv_journal;

void inv_journal_init(inv_journal *j, inv_grid *g);
void inv_journal_reset(inv_journal *j);

// open/close an edit group. everything recorded between begin and end undoes
// as one step. nesting just keeps the same group (no counter), which is fine.
void inv_journal_begin(inv_journal *j);
void inv_journal_end(inv_journal *j);

// record that `slot` is about to change: pass its current contents as `before`.
// call this *before* you mutate the slot. the `after` is filled lazily on the
// next record or undo by re-reading the live grid, so callers don't have to.
void inv_journal_record(inv_journal *j, int slot, inv_stack before);

// undo the most recent group. returns the number of slots restored (0 if the
// journal is empty). after an undo those entries are gone (no redo stack — kept
// it simple on purpose).
int  inv_journal_undo(inv_journal *j);

int  inv_journal_can_undo(const inv_journal *j);

#endif
