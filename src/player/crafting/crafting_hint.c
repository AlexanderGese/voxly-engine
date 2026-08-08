#include "crafting_hint.h"
#include "crafting_grid.h"
#include "crafting_book.h"
#include "crafting_match.h"
#include <string.h>

// build (id -> count) histograms for grid and recipe, then diff them. for the
// recipe we count one of each listed ingredient (shaped patterns flatten to the
// same thing for hint purposes; position is irrelevant here).

static void grid_hist(const craft_grid *g, int h[256]) {
    memset(h, 0, sizeof(int) * 256);
    for (int i = 0; i < CRAFT_GRID_CELLS; i++) {
        const craft_stack *s = &g->cell[i];
        if (craft_stack_empty(s)) continue;
        h[s->id] += s->count;
    }
}

static void recipe_hist(const craft_recipe *r, int h[256]) {
    memset(h, 0, sizeof(int) * 256);
    if (r->kind == CRAFT_SHAPED) {
        for (int i = 0; i < r->w * r->h; i++)
            if (r->pattern[i] != BLOCK_AIR) h[r->pattern[i]]++;
    } else {
        for (int i = 0; i < r->ing_n; i++) h[r->ing[i]]++;
    }
}

// returns total distance; splits the count into missing (need to add) and
// extra (need to remove) via the out params if non-NULL.
static int hist_distance(const int gh[256], const int rh[256],
                         int *missing, int *extra) {
    int miss = 0, ext = 0;
    for (int i = 0; i < 256; i++) {
        int d = rh[i] - gh[i];
        if (d > 0) miss += d;        // recipe wants more of i
        else if (d < 0) ext += -d;   // grid has surplus of i
    }
    if (missing) *missing = miss;
    if (extra) *extra = ext;
    return miss + ext;
}

int craft_hint_distance(const craft_grid *g, const craft_recipe *r) {
    int gh[256], rh[256];
    grid_hist(g, gh);
    recipe_hist(r, rh);
    return hist_distance(gh, rh, NULL, NULL);
}

// insertion into a small sorted-by-distance buffer. keeps the best `cap`.
static void hint_insert(craft_hint *out, int *n, int cap, craft_hint h) {
    if (*n < cap) {
        int i = *n;
        while (i > 0 && out[i - 1].distance > h.distance) {
            out[i] = out[i - 1];
            i--;
        }
        out[i] = h;
        (*n)++;
        return;
    }
    // full: only displace the worst if we beat it.
    if (h.distance >= out[cap - 1].distance) return;
    int i = cap - 1;
    while (i > 0 && out[i - 1].distance > h.distance) {
        out[i] = out[i - 1];
        i--;
    }
    out[i] = h;
}

int craft_hint_nearest(const craft_grid *g, craft_hint *out, int cap,
                       int unlocked_only) {
    if (cap <= 0) return 0;
    int total = craft_book_count();
    int gh[256];
    grid_hist(g, gh);

    int n = 0;
    for (int i = 0; i < total; i++) {
        if (unlocked_only && !craft_book_is_unlocked(i)) continue;
        const craft_recipe *r = craft_book_get(i);
        if (!r) continue;
        // skip exact matches, those aren't "hints".
        if (craft_match(g, r)) continue;

        int rh[256];
        recipe_hist(r, rh);
        craft_hint h;
        h.recipe_id = i;
        h.distance = hist_distance(gh, rh, &h.missing, &h.extra);
        if (h.distance == 0) continue;   // multiset-equal but shaped mismatch
        hint_insert(out, &n, cap, h);
    }
    return n;
}

int craft_hint_closest(const craft_grid *g, int unlocked_only) {
    craft_hint h;
    int n = craft_hint_nearest(g, &h, 1, unlocked_only);
    return n ? h.recipe_id : -1;
}

int craft_hint_missing_item(const craft_grid *g, int recipe_id,
                            block_id *out_id) {
    const craft_recipe *r = craft_book_get(recipe_id);
    if (!r) { if (out_id) *out_id = BLOCK_AIR; return 0; }

    int gh[256], rh[256];
    grid_hist(g, gh);
    recipe_hist(r, rh);

    int worst_id = BLOCK_AIR, worst = 0;
    for (int i = 0; i < 256; i++) {
        int d = rh[i] - gh[i];
        if (d > worst) { worst = d; worst_id = i; }
    }
    if (out_id) *out_id = (block_id)worst_id;
    return worst;
}
