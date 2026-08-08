#ifndef PLAYER_CRAFTING_RESOLVE_H
#define PLAYER_CRAFTING_RESOLVE_H
#include "crafting_types.h"
// the substitution-aware book lookup. the plain craft_book_find matches the
// grid literally; this one first normalizes the grid through the equivalence
// table (any-planks, any-stone, ...) so a recipe written against the canonical
// id still fires for the variant the player actually placed.
//
// it works on a scratch copy so the real working grid is never mutated. this is
// the find the crafting table should call once equivalence groups are in play;
// it falls back to the literal match when nothing fuzzy hits, so exact recipes
// keep priority.
// find the recipe the grid satisfies, allowing ingredient substitutions.
// returns the recipe id or -1. exact (literal) matches are preferred over
// substituted ones so a recipe that wants the *specific* variant still wins.
int craft_resolve_find(const craft_grid *g);
// like craft_resolve_find but restricted to unlocked recipes.
int craft_resolve_find_unlocked(const craft_grid *g);
// does the grid satisfy recipe `id` once substitutions are applied? thin
// wrapper used by tests and the hint overlay.
int craft_resolve_matches(const craft_grid *g, int recipe_id);
// returns 1 if the grid only matches *because* of a substitution (i.e. the
// literal match fails but the normalized one succeeds). drives a little
// "substituted" marker in the ui so players know it wasn't the exact recipe.
int craft_resolve_was_substituted(const craft_grid *g, int recipe_id);
#endif
