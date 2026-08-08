#ifndef PLAYER_CRAFTING_EQUIV_H
#define PLAYER_CRAFTING_EQUIV_H

#include "crafting_types.h"

// ingredient equivalence groups. a recipe pattern can list one canonical id and
// still accept any block in that id's group. classic case: "any planks" or "any
// stone-ish block". each block belongs to at most one group; the group is named
// by its canonical (representative) block id.
//
// the matcher proper doesn't know about this; we expose a normalize step that
// rewrites a grid (a copy) into canonical ids before matching, so the existing
// craft_match keeps working unchanged.

#define CRAFT_EQUIV_GROUPS  16

void craft_equiv_init(void);

// put `member` into the group represented by `canonical`. a block already in a
// group is moved. canonical blocks implicitly belong to their own group.
void craft_equiv_join(block_id canonical, block_id member);

// the canonical id for `id` (itself if it's not grouped or is the rep).
block_id craft_equiv_canon(block_id id);

// are these two ids interchangeable for crafting?
int  craft_equiv_same(block_id a, block_id b);

// rewrite every cell of `g` to its canonical id in place. counts are preserved.
// call on a *copy* of the working grid before matching so a player's real
// items aren't mutated. returns how many cells were rewritten.
struct craft_grid;
int  craft_equiv_normalize(struct craft_grid *g);

// number of distinct non-trivial groups (groups with >1 member).
int  craft_equiv_group_count(void);

#endif
