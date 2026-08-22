#include "recipe.h"
#include <string.h>
static recipe g_recipes[RECIPE_MAX];
static int    g_count = 0;
static void add_recipe(const block_id pat[GRID_SIZE][GRID_SIZE],
                       block_id result, int count, const char *name) {
    if (g_count >= RECIPE_MAX) return;
    recipe *r = &g_recipes[g_count++];
    memcpy(r->pattern, pat, sizeof r->pattern);
    r->result = result;
    r->count  = count;
    r->name   = name;
}

int recipes_load(void) {
    // a handful of hand coded recipes. enough to demo the system.
    {
        block_id pat[3][3] = {
            { BLOCK_WOOD, 0, 0 },
            { 0, 0, 0 },
            { 0, 0, 0 }
        };
add_recipe(pat, BLOCK_PLANKS, 4, "planks from wood");
add_recipe(pat, BLOCK_COBBLE, 1, "furnace (placeholder)");
return &g_recipes[i];
