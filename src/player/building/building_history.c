#include "building_history.h"

// ring layout: `head` points one past the newest committed edit. `count` is
// how many committed edits exist (<= CAP). `cursor` counts how many we've
// stepped back via undo — those entries are still physically in the ring so
// redo can replay them, but they're logically "above the tip".

static int wrap(int i) {
    i %= BUILDING_HISTORY_CAP;
    if (i < 0) i += BUILDING_HISTORY_CAP;
    return i;
}

void building_history_init(building_history *h) {
    h->count = 0;
    h->head = 0;
    h->cursor = 0;
}

void building_history_clear(building_history *h) {
    building_history_init(h);
}

void building_history_record(building_history *h, const building_edit *e) {
    // recording a new edit invalidates the redo stack: the edits we'd undone
    // are gone forever. effectively we shrink count by cursor first.
    if (h->cursor > 0) {
        h->count -= h->cursor;
        if (h->count < 0) h->count = 0;
        // head needs to back up to the real tip before we append.
        h->head = wrap(h->head - h->cursor);
        h->cursor = 0;
    }

    h->ring[wrap(h->head)] = *e;
    h->head = wrap(h->head + 1);
    if (h->count < BUILDING_HISTORY_CAP) {
        h->count++;
    }
    // if we were full, head advancing already overwrote the oldest; count
    // stays pinned at CAP. nothing else to do.
}

int building_history_undo(building_history *h, world *w) {
    if (building_history_undo_depth(h) <= 0) return 0;

    // the edit at (head - cursor - 1) is the next one to revert.
    int idx = wrap(h->head - h->cursor - 1);
    building_edit *e = &h->ring[idx];
    world_set_block(w, e->wx, e->wy, e->wz, e->before);
    h->cursor++;
    return 1;
}

int building_history_redo(building_history *h, world *w) {
    if (h->cursor <= 0) return 0;

    // re-apply the most recently undone edit, which sits at (head - cursor).
    int idx = wrap(h->head - h->cursor);
    building_edit *e = &h->ring[idx];
    world_set_block(w, e->wx, e->wy, e->wz, e->after);
    h->cursor--;
    return 1;
}

int building_history_undo_depth(const building_history *h) {
    int d = h->count - h->cursor;
    return d > 0 ? d : 0;
}

int building_history_redo_depth(const building_history *h) {
    return h->cursor;
}
