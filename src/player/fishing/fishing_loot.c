#include "fishing_loot.h"
#include <stddef.h>
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
float roll = fishing_rng_float(r);
if (roll < treasure)        return CATCH_TREASURE;
if (roll < treasure + junk) return CATCH_JUNK;
(void)fish;
return CATCH_FISH;
return fishing_loot_roll_entry(r, cat);
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
