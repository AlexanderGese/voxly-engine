#ifndef PLAYER_CRAFTING_BUILDER_H
#define PLAYER_CRAFTING_BUILDER_H
#include "crafting_types.h"
// small fluent-ish helper so the recipe table reads like data instead of a
// wall of struct initializers. you build into a craft_recipe and then hand it
// to craft_book_add. the builder also fills the `ing[]` list for shaped
// recipes automatically so the book/discovery code doesnt special-case.
// start a shaped recipe from a w*h pattern. `cells` is row-major, length w*h.
// BLOCK_AIR entries mean "must be empty". computes the ingredient list.
craft_recipe craft_build_shaped(const char *name, int w, int h,
                                const block_id *cells,
                                block_id result, int yield);
// convenience for the common 3x3 case where you pass exactly 9 ids.
craft_recipe craft_build_3x3(const char *name, const block_id grid9[9],
                             block_id result, int yield);
// shapeless recipe from a flat ingredient list (each entry = one item).
craft_recipe craft_build_shapeless(const char *name,
                                   const block_id *ings, int n,
                                   block_id result, int yield);
// trim a shaped pattern down to its tight bounding box so matching's bbox
// logic lines up. called internally but exposed for tests.
void craft_pattern_normalize(craft_recipe *r);
#endif
