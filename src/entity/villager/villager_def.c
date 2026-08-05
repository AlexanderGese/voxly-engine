#include "villager_def.h"

#include <stddef.h>

// the roster. order MUST match villager_profession. i map professions onto
// whatever blocks the engine already has rather than inventing new ones —
// a lectern is a bookshelf to us, a forge is a brick, etc. close enough.
static const villager_def defs[VILLAGER_PROF_COUNT] = {
    // name           station          radius restock speed offers
    { "unemployed",   BLOCK_AIR,         0,    0,    0.45f, 0 },
    { "farmer",       BLOCK_DIRT,        8,    4,    0.50f, 2 },
    { "librarian",    BLOCK_PLANKS,      4,    3,    0.42f, 3 },
    { "blacksmith",   BLOCK_BRICK,       5,    2,    0.55f, 2 },
    { "butcher",      BLOCK_COBBLE,      6,    3,    0.50f, 2 },
    { "mason",        BLOCK_STONE,       4,    2,    0.48f, 2 },
    { "cleric",       BLOCK_GLASS,       3,    2,    0.38f, 2 },
    { "nitwit",       BLOCK_AIR,         6,    0,    0.46f, 0 },
};

const villager_def *villager_def_get(villager_profession p) {
    if (p < 0 || p >= VILLAGER_PROF_COUNT) p = VILLAGER_PROF_UNEMPLOYED;
    return &defs[p];
}

villager_profession villager_def_for_block(block_id b) {
    // air doesn't claim anyone, even though unemployed/nitwit "use" it.
    if (b == BLOCK_AIR) return VILLAGER_PROF_COUNT;
    for (int i = 0; i < VILLAGER_PROF_COUNT; i++) {
        if (defs[i].station_block == b && defs[i].station_block != BLOCK_AIR)
            return (villager_profession)i;
    }
    return VILLAGER_PROF_COUNT;
}

const char *villager_def_name(villager_profession p) {
    return villager_def_get(p)->name;
}
