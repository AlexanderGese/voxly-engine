#ifndef PLAYER_BUILDING_HISTORY_H
#define PLAYER_BUILDING_HISTORY_H

#include "building_types.h"
#include "../../world/world.h"

// a small undo ring for player edits. handy for creative-mode oops and for
// the "did i really mean to break that" reflex. fixed capacity, oldest edit
// gets dropped when full. redo is supported until you make a new edit.

#define BUILDING_HISTORY_CAP 128

typedef struct {
    building_edit ring[BUILDING_HISTORY_CAP];
    int count;   // live entries in the ring (committed history)
    int head;    // index just past the newest committed edit
    int cursor;  // how many undos deep we are (0 = at the tip)
} building_history;

void building_history_init(building_history *h);

// push a freshly applied edit. drops any pending redo stack (you branched).
void building_history_record(building_history *h, const building_edit *e);

// undo one edit: writes `before` back into the world. returns 1 if it did
// something, 0 if there was nothing left to undo.
int  building_history_undo(building_history *h, world *w);

// redo one previously undone edit. returns 1 on success.
int  building_history_redo(building_history *h, world *w);

// how many undos / redos are currently available.
int  building_history_undo_depth(const building_history *h);
int  building_history_redo_depth(const building_history *h);

void building_history_clear(building_history *h);

#endif
