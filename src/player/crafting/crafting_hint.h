#ifndef PLAYER_CRAFTING_HINT_H
#define PLAYER_CRAFTING_HINT_H

#include "crafting_types.h"

// the "you're close!" helper. when the grid doesn't match anything, we score
// every recipe by how far the grid is from satisfying it and surface the
// nearest few. powers the ghost-overlay hint and the "almost there" nudge.
//
// distance is a cheap multiset edit count: how many single-item add/remove ops
// turn the grid's contents into the recipe's ingredient set. 0 == match.

typedef struct {
    int recipe_id;
    int distance;     // 0 means it already matches
    int missing;      // items you still need to add
    int extra;        // items you'd have to remove
} craft_hint;

// multiset distance between a grid and a recipe's ingredient list. ignores
// position entirely (it's a hint, not a matcher). >=0.
int craft_hint_distance(const craft_grid *g, const craft_recipe *r);

// score the whole book and copy the `cap` nearest non-matching recipes into
// `out`, sorted by distance asc. recipes that already match are skipped (use
// the real matcher for those). returns count written. if `unlocked_only` is
// set, only the player's unlocked recipes are considered.
int craft_hint_nearest(const craft_grid *g, craft_hint *out, int cap,
                       int unlocked_only);

// the single closest recipe id, or -1 if the book is empty. shortcut over
// craft_hint_nearest for the common "just give me one" case.
int craft_hint_closest(const craft_grid *g, int unlocked_only);

// for the ghost overlay: given a near-miss recipe, write the one ingredient id
// the player is most short on into *out_id and return how many are missing. 0
// means nothing's missing (you have enough, maybe too many).
int craft_hint_missing_item(const craft_grid *g, int recipe_id,
                            block_id *out_id);

#endif
