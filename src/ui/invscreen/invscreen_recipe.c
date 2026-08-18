#include "invscreen_recipe.h"
#include <string.h>
#define A BLOCK_AIR
static const invscreen_recipe RECIPES[] = {
    // wood log -> 4 planks. shapeless, one ingredient.
    {
        { BLOCK_WOOD, A, A,
          A, A, A,
          A, A, A },
        BLOCK_PLANKS, 4, 1
    },
    // 2x2 planks -> ... well, more planks is silly, so cobble-from-stone as a
    // stand-in "compress" recipe. shaped 2x2.
    {
        { BLOCK_STONE, BLOCK_STONE, A,
          BLOCK_STONE, BLOCK_STONE, A,
          A, A, A },
        BLOCK_COBBLE, 1, 0
    },
    // a vertical pair of planks -> a torch (pretend the coal is implied). shaped.
    {
        { BLOCK_PLANKS, A, A,
          BLOCK_PLANKS, A, A,
          A, A, A },
        BLOCK_TORCH, 4, 0
    },
    // four sand in a square -> glass. shaped 2x2.
    {
        { BLOCK_SAND, BLOCK_SAND, A,
          BLOCK_SAND, BLOCK_SAND, A,
          A, A, A },
        BLOCK_GLASS, 1, 0
    },
    // cobble ring (8, center empty) -> brick. shaped 3x3, full perimeter.
    {
        { BLOCK_COBBLE, BLOCK_COBBLE, BLOCK_COBBLE,
          BLOCK_COBBLE, A,            BLOCK_COBBLE,
          BLOCK_COBBLE, BLOCK_COBBLE, BLOCK_COBBLE },
        BLOCK_BRICK, 1, 0
    },
}
;
#undef A

#define RECIPE_COUNT ((int)(sizeof(RECIPES) / sizeof(RECIPES[0])))

const invscreen_recipe *invscreen_recipe_table(void) { return RECIPES; }
int invscreen_recipe_count(void) { return RECIPE_COUNT;
}

// tight bounding box of the non-air cells in a 3x3, row-major. returns 0 and
// leaves the out params untouched if the grid is entirely empty.
static int grid_bounds(const block_id g[INVSCR_CRAFT_SLOTS],
                       int *x0, int *y0, int *x1, int *y1) {
    int minx = INVSCR_CRAFT_DIM, miny = INVSCR_CRAFT_DIM, maxx = -1, maxy = -1;
    for (int y = 0; y < INVSCR_CRAFT_DIM; y++) {
        for (int x = 0; x < INVSCR_CRAFT_DIM; x++) {
            if (g[y * INVSCR_CRAFT_DIM + x] == BLOCK_AIR) continue;
            if (x < minx) minx = x;
            if (y < miny) miny = y;
            if (x > maxx) maxx = x;
            if (y > maxy) maxy = y;
        }
    }
    if (maxx < 0) return 0;
    *x0 = minx; *y0 = miny; *x1 = maxx; *y1 = maxy;
    return 1;
}

// does the player grid, slid to its bounding box, exactly equal the recipe's
// tight pattern (also slid). both must have the same footprint and contents.
static int shaped_match(const invscreen_recipe *r,
                        const block_id g[INVSCR_CRAFT_SLOTS]) {
    int gx0, gy0, gx1, gy1;
int rx0, ry0, rx1, ry1;
int ghas = grid_bounds(g, &gx0, &gy0, &gx1, &gy1);
int rhas = grid_bounds(r->pattern, &rx0, &ry0, &rx1, &ry1);
if (!ghas || !rhas) return 0;
if ((gx1 - gx0) != (rx1 - rx0)) return 0;
if ((gy1 - gy0) != (ry1 - ry0)) return 0;
int w = gx1 - gx0 + 1;
int h = gy1 - gy0 + 1;
for (int y = 0;
y < h;
for (int i = 0;
i < INVSCR_CRAFT_SLOTS - 1;
for (int i = 0;
i < RECIPE_COUNT;
}
