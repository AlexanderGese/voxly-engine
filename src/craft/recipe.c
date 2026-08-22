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
    }
    {
        block_id pat[3][3] = {
            { BLOCK_PLANKS, BLOCK_PLANKS, BLOCK_PLANKS },
            { 0, 0, 0 },
            { 0, 0, 0 }
        };
        add_recipe(pat, BLOCK_PLANKS, 3, "slab (placeholder)");
    }
    {
        block_id pat[3][3] = {
            { BLOCK_COBBLE, BLOCK_COBBLE, BLOCK_COBBLE },
            { BLOCK_COBBLE, 0, BLOCK_COBBLE },
            { BLOCK_COBBLE, BLOCK_COBBLE, BLOCK_COBBLE }
        };
        add_recipe(pat, BLOCK_COBBLE, 1, "furnace (placeholder)");
    }
    {
        block_id pat[3][3] = {
            { BLOCK_STONE, BLOCK_STONE, BLOCK_STONE },
            { BLOCK_STONE, BLOCK_STONE, BLOCK_STONE },
            { 0, 0, 0 }
        };
        add_recipe(pat, BLOCK_BRICK, 4, "bricks");
    }
    return g_count;
}

int recipes_count(void) { return g_count; }

const recipe *recipes_get(int i) {
    if (i < 0 || i >= g_count) return NULL;
    return &g_recipes[i];
}

const recipe *recipes_match(const block_id grid[GRID_SIZE][GRID_SIZE]) {
    // exact match including empty cells. not shapeless.
    for (int r = 0; r < g_count; r++) {
        const recipe *rec = &g_recipes[r];
        int ok = 1;
        for (int y = 0; y < 3 && ok; y++) {
            for (int x = 0; x < 3 && ok; x++) {
                if (rec->pattern[y][x] != grid[y][x]) ok = 0;
            }
        }
        if (ok) return rec;
    }
    return NULL;
}
