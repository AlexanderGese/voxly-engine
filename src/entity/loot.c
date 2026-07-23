#include "loot.h"
#include "../world/block.h"
#include "../math/rng.h"

static rng loot_rng;
static int loot_rng_init = 0;

void loot_drop(item_world *iw, const entity *e) {
    if (!loot_rng_init) { rng_init(&loot_rng, 0xDEAD1007); loot_rng_init = 1; }

    switch (e->type) {
    case ET_ZOMBIE:
        if (rng_float01(&loot_rng) < 0.5f)
            item_spawn(iw, e->pos, BLOCK_COBBLE);
        break;
    case ET_COW:
        item_spawn(iw, e->pos, BLOCK_DIRT);
        if (rng_float01(&loot_rng) < 0.3f)
            item_spawn(iw, e->pos, BLOCK_DIRT);
        break;
    case ET_PIG:
        item_spawn(iw, e->pos, BLOCK_SAND);
        break;
    case ET_SKELETON:
        item_spawn(iw, e->pos, BLOCK_COBBLE);
        item_spawn(iw, e->pos, BLOCK_COBBLE);
        break;
    case ET_SPIDER:
        if (rng_float01(&loot_rng) < 0.4f)
            item_spawn(iw, e->pos, BLOCK_GLASS);
        break;
    default:
        break;
    }
}
