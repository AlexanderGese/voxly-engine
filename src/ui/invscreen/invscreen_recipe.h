#ifndef UI_INVSCREEN_RECIPE_H
#define UI_INVSCREEN_RECIPE_H
#include "../../world/block.h"
#include "invscreen_config.h"
typedef struct {
    block_id pattern[INVSCR_CRAFT_SLOTS];  // row-major 3x3, AIR = empty cell
    block_id result;
    int      result_count;
    int      shapeless;                    // 1 = ignore arrangement
} invscreen_recipe;
const invscreen_recipe *invscreen_recipe_table(void);
int invscreen_recipe_count(void);
const invscreen_recipe *invscreen_recipe_match(const block_id grid[INVSCR_CRAFT_SLOTS]);
#endif
