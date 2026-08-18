#ifndef UI_INVSCREEN_SORT_H
#define UI_INVSCREEN_SORT_H

// backpack housekeeping: merge partial stacks of the same id, then sort the grid
// so identical items sit together and empties fall to the end. bound to a button
// in the panel chrome (the host wires the click). only ever touches the grid
// region — the hotbar, craft slots and the held cursor are left alone so muscle
// memory survives a sort.

#include "invscreen_model.h"

// merge same-id partial stacks within the grid so each id occupies the fewest
// cells. returns how many cells it managed to free up (became empty). stable in
// the sense that it pours later stacks into earlier ones, never the reverse.
int invscreen_sort_compact(invscreen_model *m);

// full tidy: compact, then sort the grid cells by a stable item order (id, then
// descending count). returns 1 if anything moved. idempotent — running it twice
// in a row is a no-op the second time.
int invscreen_sort_grid(invscreen_model *m);

#endif
