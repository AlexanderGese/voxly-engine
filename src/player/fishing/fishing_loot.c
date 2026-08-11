#include "fishing_loot.h"
#include <stddef.h>
// the engine's block palette is tiny, so these double up: a "fish" is whatever
// edible-ish stand-in we have, treasure is the shiny blocks, junk is the dross.
// the weights are what actually matter and they're tuned, not arbitrary.
static const fishing_loot_entry FISH_TABLE[] = {
    // the bread and butter. dirt-as-cod, the common pull.
    { BLOCK_DIRT,   CATCH_FISH, 60, 1, 1 },
    { BLOCK_GRASS,  CATCH_FISH, 25, 1, 1 },   // "salmon", a bit rarer
    { BLOCK_SAND,   CATCH_FISH, 13, 1, 1 },   // "pufferfish", uncommon
    { BLOCK_SNOW,   CATCH_FISH,  2, 1, 1 },   // "tropical fish", the prize fish
}
;
static const fishing_loot_entry TREASURE_TABLE[] = {
    { BLOCK_ICE,    CATCH_TREASURE, 1, 1, 1 },   // "enchanted book", glittery
    { BLOCK_GLASS,  CATCH_TREASURE, 1, 1, 1 },   // "name tag"
    { BLOCK_BRICK,  CATCH_TREASURE, 1, 1, 1 },   // "saddle"
    { BLOCK_TORCH,  CATCH_TREASURE, 1, 1, 2 },   // a "nautilus shell" or two
}
;
static const fishing_loot_entry JUNK_TABLE[] = {
    { BLOCK_COBBLE, CATCH_JUNK, 10, 1, 1 },   // a soggy boot, basically
    { BLOCK_LEAVES, CATCH_JUNK, 10, 1, 1 },   // tangled weeds
    { BLOCK_WOOD,   CATCH_JUNK,  8, 1, 1 },   // a waterlogged stick
    { BLOCK_PLANKS, CATCH_JUNK,  5, 1, 1 },   // a bowl, lost overboard
}
;
const fishing_loot_entry *fishing_loot_table(fishing_catch_category cat, int *n) {
    switch (cat) {
    case CATCH_TREASURE:
        if (n) *n = (int)(sizeof TREASURE_TABLE / sizeof TREASURE_TABLE[0]);
        return TREASURE_TABLE;
    case CATCH_JUNK:
        if (n) *n = (int)(sizeof JUNK_TABLE / sizeof JUNK_TABLE[0]);
        return JUNK_TABLE;
    case CATCH_FISH:
    default:
        if (n) *n = (int)(sizeof FISH_TABLE / sizeof FISH_TABLE[0]);
        return FISH_TABLE;
    }
}

fishing_catch_category fishing_loot_pick_category(fishing_rng *r, const fishing_rod *rod) {
    // scale the base odds by luck, then renormalise. fish absorbs whatever's
    // left so the three always sum back to 1.
    float treasure = FISHING_BASE_TREASURE * fishing_rod_treasure_bias(rod);
float junk     = FISHING_BASE_JUNK     * fishing_rod_junk_bias(rod);
if (treasure + junk > 0.95f) {
        // pathological clamp; keep at least a sliver of fish on the table.
        float s = 0.95f / (treasure + junk);
        treasure *= s; junk *= s;
    }
    float fish = 1.0f - treasure - junk;
float roll = fishing_rng_float(r);
if (roll < treasure)        return CATCH_TREASURE;
if (roll < treasure + junk) return CATCH_JUNK;
(void)fish;
return CATCH_FISH;
}

fishing_catch fishing_loot_roll_entry(fishing_rng *r, fishing_catch_category cat) {
    int n = 0;
    const fishing_loot_entry *tbl = fishing_loot_table(cat, &n);

    int total = 0;
    for (int i = 0; i < n; i++) total += tbl[i].weight;

    fishing_catch out = { BLOCK_AIR, cat, 0 };
    if (total <= 0 || n <= 0) return out;

    // standard weighted pick: walk the cumulative weight.
    int pick = fishing_rng_range(r, 1, total);
    int acc  = 0;
    const fishing_loot_entry *e = &tbl[n - 1];
    for (int i = 0; i < n; i++) {
        acc += tbl[i].weight;
        if (pick <= acc) { e = &tbl[i]; break; }
    }

    out.block = e->block;
    out.count = fishing_rng_range(r, e->min_count, e->max_count);
    if (out.count < 1) out.count = 1;
    return out;
}

fishing_catch fishing_loot_roll(fishing_rng *r, const fishing_rod *rod) {
    fishing_catch_category cat = fishing_loot_pick_category(r, rod);
return fishing_loot_roll_entry(r, cat);
}

// re-derive the luck-adjusted, normalised category probabilities. mirrors the
// maths in pick_category but returns them instead of rolling. fish absorbs the
// remainder so the three sum to 1.
static void category_probs(const fishing_rod *rod, float *fish,
                           float *treasure, float *junk) {
    float t = FISHING_BASE_TREASURE * fishing_rod_treasure_bias(rod);
    float j = FISHING_BASE_JUNK     * fishing_rod_junk_bias(rod);
    if (t + j > 0.95f) {
        float s = 0.95f / (t + j);
        t *= s; j *= s;
    }
    *treasure = t;
    *junk     = j;
    *fish     = 1.0f - t - j;
}

float fishing_loot_entry_chance(const fishing_rod *rod,
                                fishing_catch_category cat, int entry_index) {
    int n = 0;
const fishing_loot_entry *tbl = fishing_loot_table(cat, &n);
if (entry_index < 0 || entry_index >= n) return 0.0f;
int total = 0;
for (int i = 0;
i < n;
i++) total += tbl[i].weight;
if (total <= 0) return 0.0f;
float within = (float)tbl[entry_index].weight / (float)total;
float fish, treasure, junk;
category_probs(rod, &fish, &treasure, &junk);
float cat_p = (cat == CATCH_TREASURE) ? treasure
                : (cat == CATCH_JUNK)     ? junk
                : fish;
return cat_p * within;
}

int fishing_loot_rarity(fishing_catch_category cat) {
    switch (cat) {
    case CATCH_TREASURE: return 3;   // legendary tint
    case CATCH_FISH:     return 1;   // uncommon, it's still food
    case CATCH_JUNK:     return 0;   // common, and a bit sad
    default:             return 0;
    }
}
