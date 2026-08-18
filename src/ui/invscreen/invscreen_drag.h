#ifndef UI_INVSCREEN_DRAG_H
#define UI_INVSCREEN_DRAG_H

// the drag/pickup state machine. this is where all the fiddly inventory
// interaction lives: left-click to grab/drop/stack, right-click to split or
// drop one, and the "paint" drag where you hold a stack and sweep across empty
// slots to spread it evenly. classic minecraft feel.
//
// it operates entirely on the model's slot array through absolute indices. the
// cursor-held slot is just another slot (the tail one) so most operations are
// expressed as transfers between two slots.

#include "invscreen_model.h"

// what the drag layer wants the caller to know happened this step, so the
// controller can re-resolve crafting / play a click only when state moved.
enum {
    INVSCR_DRAG_NONE = 0,
    INVSCR_DRAG_GRABBED,   // picked a stack up onto the cursor
    INVSCR_DRAG_PLACED,    // dropped / merged into a slot
    INVSCR_DRAG_SPLIT,     // right-click split
    INVSCR_DRAG_PAINTED,   // distributed one to a slot during a sweep
};

typedef struct {
    // left-button "paint" sweep. while active we remember which slots we've
    // already dropped into so the sweep doesn't double-dip the same cell.
    int painting;
    int paint_count;                       // items in hand when sweep began
    int paint_visited[INVSCR_SLOT_TOTAL];  // 1 if we dropped here this sweep
    int paint_n;                           // how many distinct cells visited
} invscreen_drag;

void invscreen_drag_init(invscreen_drag *d);

// left click on `slot` (absolute index, or INVSCR_NO_SLOT for "outside panel").
// shift toggles quick-move. returns one of the INVSCR_DRAG_* result codes.
int invscreen_drag_left(invscreen_drag *d, invscreen_model *m, int slot, int shift);

// right click on `slot`. half-pickup when nothing held, drop-one when holding.
int invscreen_drag_right(invscreen_drag *d, invscreen_model *m, int slot);

// while the left button stays down and we're holding a stack, call this with the
// slot currently under the cursor to keep the paint sweep going. cheap no-op
// when not painting or when the slot was already visited.
int invscreen_drag_paint(invscreen_drag *d, invscreen_model *m, int slot);

// left button released — finalize a paint sweep (it's already applied, this just
// clears the sweep bookkeeping).
void invscreen_drag_release(invscreen_drag *d);

#endif
