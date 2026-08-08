#ifndef PLAYER_CRAFTING_MATCH_H
#define PLAYER_CRAFTING_MATCH_H

#include "crafting_types.h"

// the actual matching logic. given a grid and a recipe, decide whether the
// grid satisfies it. shaped recipes are position-sensitive but translation
// invariant (we slide the pattern around the grid's bounding box). shapeless
// recipes only care about the multiset of ingredients.

// returns 1 if `g` matches `r`. does not mutate anything.
int craft_match(const craft_grid *g, const craft_recipe *r);

// shaped-only test. exposed for the book/tests; craft_match dispatches here.
int craft_match_shaped(const craft_grid *g, const craft_recipe *r);

// shapeless-only test.
int craft_match_shapeless(const craft_grid *g, const craft_recipe *r);

// also offer a mirrored shaped match (left-right flip) since a fair number of
// recipes read the same flipped and players expect that to work.
int craft_match_shaped_mirrored(const craft_grid *g, const craft_recipe *r);

#endif
