#include "crafting_resolve.h"
#include "crafting_grid.h"
#include "crafting_match.h"
#include "crafting_book.h"
#include "crafting_equiv.h"

// most of the work is just "copy the grid, normalize it, run the normal
// matcher". craft_grid is a flat value type so the copy is a struct assign.

static int literal_match(const craft_grid *g, int id) {
    const craft_recipe *r = craft_book_get(id);
    return r ? craft_match(g, r) : 0;
}

static int normalized_match(const craft_grid *g, int id) {
    const craft_recipe *r = craft_book_get(id);
    if (!r) return 0;
    craft_grid tmp = *g;
    craft_equiv_normalize(&tmp);
    // the recipe pattern itself is authored in canonical ids, but normalize it
    // too in case someone registered a variant id by hand. cheap insurance.
    craft_recipe rn = *r;
    if (rn.kind == CRAFT_SHAPED) {
        for (int i = 0; i < CRAFT_GRID_CELLS; i++)
            if (rn.pattern[i] != BLOCK_AIR)
                rn.pattern[i] = craft_equiv_canon(rn.pattern[i]);
    }
    for (int i = 0; i < rn.ing_n; i++)
        rn.ing[i] = craft_equiv_canon(rn.ing[i]);
    return craft_match(&tmp, &rn);
}

int craft_resolve_matches(const craft_grid *g, int recipe_id) {
    if (literal_match(g, recipe_id)) return 1;
    return normalized_match(g, recipe_id);
}

int craft_resolve_was_substituted(const craft_grid *g, int recipe_id) {
    // substituted == fuzzy hits but literal doesn't.
    if (literal_match(g, recipe_id)) return 0;
    return normalized_match(g, recipe_id);
}

int craft_resolve_find(const craft_grid *g) {
    int total = craft_book_count();
    // first pass: exact matches win outright, scanning in id order.
    for (int i = 0; i < total; i++)
        if (literal_match(g, i)) return i;
    // second pass: substituted matches.
    for (int i = 0; i < total; i++)
        if (normalized_match(g, i)) return i;
    return -1;
}

int craft_resolve_find_unlocked(const craft_grid *g) {
    int total = craft_book_count();
    for (int i = 0; i < total; i++) {
        if (!craft_book_is_unlocked(i)) continue;
        if (literal_match(g, i)) return i;
    }
    for (int i = 0; i < total; i++) {
        if (!craft_book_is_unlocked(i)) continue;
        if (normalized_match(g, i)) return i;
    }
    return -1;
}
