#include "crafting_builder.h"
#include <string.h>

static void fill_ingredients(craft_recipe *r, const block_id *cells, int n) {
    r->ing_n = 0;
    for (int i = 0; i < n; i++) {
        if (cells[i] == BLOCK_AIR) continue;
        if (r->ing_n < CRAFT_MAX_INGREDIENTS)
            r->ing[r->ing_n++] = cells[i];
    }
}

void craft_pattern_normalize(craft_recipe *r) {
    // find tight bbox of the pattern within its declared w*h.
    int minx = r->w, miny = r->h, maxx = -1, maxy = -1;
    for (int y = 0; y < r->h; y++) {
        for (int x = 0; x < r->w; x++) {
            if (r->pattern[y * r->w + x] == BLOCK_AIR) continue;
            if (x < minx) minx = x;
            if (y < miny) miny = y;
            if (x > maxx) maxx = x;
            if (y > maxy) maxy = y;
        }
    }
    if (maxx < 0) return;  // all-air pattern, leave it (degenerate)

    int nw = maxx - minx + 1;
    int nh = maxy - miny + 1;
    if (nw == r->w && nh == r->h) return;  // already tight

    block_id tmp[CRAFT_GRID_CELLS];
    for (int y = 0; y < nh; y++)
        for (int x = 0; x < nw; x++)
            tmp[y * nw + x] = r->pattern[(miny + y) * r->w + (minx + x)];

    memset(r->pattern, BLOCK_AIR, sizeof r->pattern);
    memcpy(r->pattern, tmp, (size_t)nw * nh * sizeof(block_id));
    r->w = nw;
    r->h = nh;
}

craft_recipe craft_build_shaped(const char *name, int w, int h,
                                const block_id *cells,
                                block_id result, int yield) {
    craft_recipe r;
    memset(&r, 0, sizeof r);
    r.kind = CRAFT_SHAPED;
    r.name = name;
    if (w < 1) w = 1; if (w > CRAFT_GRID_MAX) w = CRAFT_GRID_MAX;
    if (h < 1) h = 1; if (h > CRAFT_GRID_MAX) h = CRAFT_GRID_MAX;
    r.w = w; r.h = h;
    memset(r.pattern, BLOCK_AIR, sizeof r.pattern);
    for (int i = 0; i < w * h; i++) r.pattern[i] = cells[i];
    fill_ingredients(&r, cells, w * h);
    r.result = craft_stack_make(result, yield);
    craft_pattern_normalize(&r);
    return r;
}

craft_recipe craft_build_3x3(const char *name, const block_id grid9[9],
                             block_id result, int yield) {
    return craft_build_shaped(name, 3, 3, grid9, result, yield);
}

craft_recipe craft_build_shapeless(const char *name,
                                   const block_id *ings, int n,
                                   block_id result, int yield) {
    craft_recipe r;
    memset(&r, 0, sizeof r);
    r.kind = CRAFT_SHAPELESS;
    r.name = name;
    r.w = 0; r.h = 0;
    memset(r.pattern, BLOCK_AIR, sizeof r.pattern);
    if (n > CRAFT_MAX_INGREDIENTS) n = CRAFT_MAX_INGREDIENTS;
    r.ing_n = n;
    for (int i = 0; i < n; i++) r.ing[i] = ings[i];
    r.result = craft_stack_make(result, yield);
    return r;
}
