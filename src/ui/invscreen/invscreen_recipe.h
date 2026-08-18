#ifndef UI_INVSCREEN_RECIPE_H
#define UI_INVSCREEN_RECIPE_H

// crafting recipes + the matcher. recipes are 3x3 shaped patterns over block
// ids, with BLOCK_AIR meaning "this cell must be empty". a recipe can be marked
// shapeless, in which case position doesn't matter, only the multiset of
// ingredients. there's a small built-in table; nothing reads recipes from disk
// yet (that's a someday problem).

#include "../../world/block.h"
#include "invscreen_config.h"

typedef struct {
    block_id pattern[INVSCR_CRAFT_SLOTS];  // row-major 3x3, AIR = empty cell
    block_id result;
    int      result_count;
    int      shapeless;                    // 1 = ignore arrangement
} invscreen_recipe;

// the static recipe table. count via invscreen_recipe_count().
const invscreen_recipe *invscreen_recipe_table(void);
int invscreen_recipe_count(void);

// match a 3x3 grid (row-major block ids, AIR = empty) against the table. returns
// the matching recipe or NULL. handles shaped recipes that don't fill the whole
// grid by sliding the pattern to its tight bounding box before comparing, so a
// 2x2 recipe matches no matter which corner the player placed it in.
const invscreen_recipe *invscreen_recipe_match(const block_id grid[INVSCR_CRAFT_SLOTS]);

#endif
