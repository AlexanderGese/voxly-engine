#include "invscreen_recipe.h"

#include <string.h>

// the built-in recipe table. row-major 3x3 patterns; BLOCK_AIR is an empty
// cell. shaped recipes are stored in their tight form (top-left anchored) and
// the matcher slides the player grid to its own bounding box before comparing,
// so where on the bench they laid it out doesn't matter. shapeless ones only
// care about the multiset of ingredients.
//
// nothing exotic in here yet, just enough to prove the pipeline works end to
// end. add more when the block set grows.
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
};
#undef A

#define RECIPE_COUNT ((int)(sizeof(RECIPES) / sizeof(RECIPES[0])))

const invscreen_recipe *invscreen_recipe_table(void) { return RECIPES; }
int invscreen_recipe_count(void) { return RECIPE_COUNT; }

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

    // footprints must be identical in size or it's not the same shape.
    if ((gx1 - gx0) != (rx1 - rx0)) return 0;
    if ((gy1 - gy0) != (ry1 - ry0)) return 0;

    int w = gx1 - gx0 + 1;
    int h = gy1 - gy0 + 1;
    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            block_id gb = g[(gy0 + y) * INVSCR_CRAFT_DIM + (gx0 + x)];
            block_id rb = r->pattern[(ry0 + y) * INVSCR_CRAFT_DIM + (rx0 + x)];
            if (gb != rb) return 0;
        }
    }
    return 1;
}

// shapeless: same multiset of non-air ids regardless of position. we count one
// side and decrement against the other, bailing on any mismatch.
static int shapeless_match(const invscreen_recipe *r,
                           const block_id g[INVSCR_CRAFT_SLOTS]) {
    int need[BLOCK_COUNT] = { 0 };
    int have[BLOCK_COUNT] = { 0 };
    int nneed = 0, nhave = 0;
    for (int i = 0; i < INVSCR_CRAFT_SLOTS; i++) {
        if (r->pattern[i] != BLOCK_AIR) { need[r->pattern[i]]++; nneed++; }
        if (g[i] != BLOCK_AIR)          { have[g[i]]++;          nhave++; }
    }
    if (nneed != nhave) return 0;
    for (int b = 0; b < BLOCK_COUNT; b++)
        if (need[b] != have[b]) return 0;
    return 1;
}

const invscreen_recipe *invscreen_recipe_match(const block_id grid[INVSCR_CRAFT_SLOTS]) {
    // empty grid never matches. cheap early out before the table walk.
    int any = 0;
    for (int i = 0; i < INVSCR_CRAFT_SLOTS; i++)
        if (grid[i] != BLOCK_AIR) { any = 1; break; }
    if (!any) return NULL;

    for (int i = 0; i < RECIPE_COUNT; i++) {
        const invscreen_recipe *r = &RECIPES[i];
        int ok = r->shapeless ? shapeless_match(r, grid)
                              : shaped_match(r, grid);
        if (ok) return r;
    }
    return NULL;
}
