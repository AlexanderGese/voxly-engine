#include "inv_journal.h"
#include "inv_stack.h"
#include <string.h>

void inv_journal_init(inv_journal *j, inv_grid *g) {
    memset(j, 0, sizeof *j);
    j->grid  = g;
    j->group = 1;       // group 0 is reserved for "no group yet"
}

void inv_journal_reset(inv_journal *j) {
    j->head     = 0;
    j->len      = 0;
    j->in_group = 0;
    j->group    = 1;
}

void inv_journal_begin(inv_journal *j) {
    if (j->in_group) return;    // already grouping, keep the same id
    j->in_group = 1;
    j->group++;
    if (j->group == 0) j->group = 1;   // skip the reserved id on wrap
}

void inv_journal_end(inv_journal *j) {
    j->in_group = 0;
}

// the oldest entry's index in the ring, given head/len.
static int tail_index(const inv_journal *j) {
    return (j->head - j->len + INV_JOURNAL_DEPTH * 2) % INV_JOURNAL_DEPTH;
}

void inv_journal_record(inv_journal *j, int slot, inv_stack before) {
    if (!inv_grid_in_bounds(j->grid, slot)) return;

    // patch the previous entry's `after` from the live grid first. we record
    // `before` up front but only know `after` once the next edit lands or an
    // undo runs; snapshotting the current slot here keeps it honest cheaply.
    if (j->len > 0) {
        int prev = (j->head - 1 + INV_JOURNAL_DEPTH) % INV_JOURNAL_DEPTH;
        inv_journal_entry *pe = &j->ring[prev];
        pe->after = *inv_grid_cat(j->grid, pe->slot);
    }

    inv_journal_entry *e = &j->ring[j->head];
    e->slot   = slot;
    e->before = before;
    e->after  = before;     // provisional, fixed up on the next record/undo
    e->group  = j->in_group ? j->group : ++j->group;

    j->head = (j->head + 1) % INV_JOURNAL_DEPTH;
    if (j->len < INV_JOURNAL_DEPTH) j->len++;
    // if we wrapped, the tail entry just got clobbered; len stays pinned at DEPTH
    // and the oldest edit is silently unrecoverable. acceptable for an undo ring.
}

int inv_journal_can_undo(const inv_journal *j) {
    return j->len > 0;
}

int inv_journal_undo(inv_journal *j) {
    if (j->len == 0) return 0;

    // the group to undo is whatever the newest entry belongs to.
    int newest = (j->head - 1 + INV_JOURNAL_DEPTH) % INV_JOURNAL_DEPTH;
    uint32_t g = j->ring[newest].group;

    // before restoring, make sure the newest entry's `after` reflects the live
    // grid (it may still be provisional if no edit followed it).
    j->ring[newest].after = *inv_grid_cat(j->grid, j->ring[newest].slot);

    int restored = 0;
    int tail = tail_index(j);
    // pop entries off the head while they share the group, restoring `before`.
    while (j->len > 0) {
        int i = (j->head - 1 + INV_JOURNAL_DEPTH) % INV_JOURNAL_DEPTH;
        inv_journal_entry *e = &j->ring[i];
        if (e->group != g) break;

        inv_stack *live = inv_grid_at(j->grid, e->slot);
        if (live) { *live = e->before; restored++; }

        j->head = i;
        j->len--;
        if (i == tail) break;   // drained the whole ring
    }
    return restored;
}
