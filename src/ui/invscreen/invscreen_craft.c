#include "invscreen_craft.h"

#include <string.h>

void invscreen_craft_init(invscreen_craft *cr) {
    cr->recipe = NULL;
    cr->dirty  = 1;   // force a first resolve
}

void invscreen_craft_touch(invscreen_craft *cr) {
    cr->dirty = 1;
}

// read the 3x3 craft-in region of the model into a flat block_id grid. empty
// slots come out as BLOCK_AIR, which is exactly what the matcher wants.
static void read_grid(const invscreen_model *m, block_id grid[INVSCR_CRAFT_SLOTS]) {
    int base = invscreen_model_region_base(INVSCR_REGION_CRAFT_IN);
    for (int i = 0; i < INVSCR_CRAFT_SLOTS; i++) {
        const invscreen_slot *s = invscreen_model_at_c(m, base + i);
        grid[i] = (s && !invscreen_slot_is_empty(s)) ? s->block : BLOCK_AIR;
    }
}

int invscreen_craft_resolve(invscreen_craft *cr, invscreen_model *m) {
    if (!cr->dirty) return 0;
    cr->dirty = 0;

    block_id grid[INVSCR_CRAFT_SLOTS];
    read_grid(m, grid);

    const invscreen_recipe *r = invscreen_recipe_match(grid);
    invscreen_slot *out = invscreen_model_at(
        m, invscreen_model_region_base(INVSCR_REGION_CRAFT_OUT));

    // remember what was there so we can tell the caller if it actually changed.
    invscreen_slot prev = out ? *out : invscreen_slot_empty();

    if (r) {
        *out = invscreen_slot_make(r->result, r->result_count);
    } else {
        *out = invscreen_slot_empty();
    }
    cr->recipe = r;

    return prev.block != out->block || prev.count != out->count;
}

int invscreen_craft_consume(invscreen_craft *cr, invscreen_model *m) {
    if (!cr->recipe) return 0;

    // re-validate before touching anything: the inputs may have shifted since
    // the last resolve (e.g. the player yanked a slot out). cheaper to re-match
    // than to thread invalidation through every drag path.
    block_id grid[INVSCR_CRAFT_SLOTS];
    read_grid(m, grid);
    if (invscreen_recipe_match(grid) != cr->recipe) return 0;

    int base = invscreen_model_region_base(INVSCR_REGION_CRAFT_IN);
    for (int i = 0; i < INVSCR_CRAFT_SLOTS; i++) {
        invscreen_slot *s = invscreen_model_at(m, base + i);
        if (!s || invscreen_slot_is_empty(s)) continue;
        s->count--;
        if (s->count <= 0) *s = invscreen_slot_empty();
    }

    // inputs changed, so the next resolve has to re-run.
    cr->dirty = 1;
    return 1;
}

int invscreen_craft_batches(const invscreen_craft *cr, const invscreen_model *m) {
    if (!cr->recipe) return 0;

    // the limiting input is the smallest non-empty craft-in stack. every
    // non-air cell in the recipe consumes exactly one per batch.
    int base = invscreen_model_region_base(INVSCR_REGION_CRAFT_IN);
    int limit = INVSCR_STACK_MAX + 1;   // sentinel high
    for (int i = 0; i < INVSCR_CRAFT_SLOTS; i++) {
        const invscreen_slot *s = invscreen_model_at_c(m, base + i);
        if (!s || invscreen_slot_is_empty(s)) continue;
        if (s->count < limit) limit = s->count;
    }
    return limit > INVSCR_STACK_MAX ? 0 : limit;
}
