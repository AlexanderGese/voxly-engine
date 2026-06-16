#include "farming_harvest.h"
#include "farming_def.h"
#include "farming_block.h"
#include "farming_growth.h"
#include "../block.h"
#include <stddef.h>

// roll an inclusive [lo,hi] off a stream rng.
static int roll_range(farming_rng *r, int lo, int hi) {
    if (hi <= lo) return lo;
    return lo + (int)(farming_rng_next(r) % (uint32_t)(hi - lo + 1));
}

// binomial bonus: `rolls` independent successes at probability `p`. this is the
// classic "0..3 extra wheat" distribution.
static int binomial_bonus(farming_rng *r, int rolls, float p) {
    int n = 0;
    for (int i = 0; i < rolls; i++)
        if (farming_rng_chance(r, p)) n++;
    return n;
}

farming_yield farming_harvest_roll(const farming_def *def, farming_rng *rng) {
    farming_yield y = {0, 0, 0, 0};

    y.produce_count = roll_range(rng, def->produce_min, def->produce_max);
    y.produce_count += binomial_bonus(rng, def->bonus_rolls, def->bonus_chance);

    y.seed_count = roll_range(rng, def->seed_min, def->seed_max);
    // seeds also enjoy a slim bonus roll so replant stock keeps up with food.
    if (def->seed_max > 0)
        y.seed_count += binomial_bonus(rng, def->bonus_rolls, def->bonus_chance);

    y.xp = def->xp;
    y.block_drop = 0;
    return y;
}

farming_yield farming_harvest_crop(world *w, int wx, int wy, int wz,
                                   const farming_crop *crop, farming_rng *rng) {
    farming_yield empty = {0, 0, 0, 0};

    block_id here = world_get_block(w, wx, wy, wz);
    if (!farming_block_is_crop(here)) return empty;

    farming_crop_kind kind = farming_crop_for_block(here);
    const farming_def *def = farming_def_get(kind);

    // stems never produce on harvest; their fruit is a separate block. breaking
    // a stem yields its seeds only.
    if (def->is_stem) {
        farming_yield y = {0, 0, 0, 0};
        y.seed_count = roll_range(rng, def->seed_min, def->seed_max);
        y.xp = def->xp;
        world_set_block(w, wx, wy, wz, BLOCK_AIR);
        return y;
    }

    // not mature -> uproot for a pittance instead of a full yield.
    if (!crop || !farming_growth_is_mature(crop, def)) {
        farming_yield y = {0, 0, 0, 0};
        y.seed_count = 1;
        world_set_block(w, wx, wy, wz, BLOCK_AIR);
        return y;
    }

    farming_yield y = farming_harvest_roll(def, rng);
    world_set_block(w, wx, wy, wz, BLOCK_AIR);
    return y;
}

farming_yield farming_harvest_fruit(world *w, int wx, int wy, int wz) {
    farming_yield y = {0, 0, 0, 0};

    block_id here = world_get_block(w, wx, wy, wz);
    if (here == FARMING_BLOCK_MELON_FRUIT) {
        y.block_drop = FARMING_BLOCK_MELON_FRUIT;
        y.produce_count = 1; // a chopped fruit also yields a few slices, eh
        y.xp = 1;
        world_set_block(w, wx, wy, wz, BLOCK_AIR);
    } else if (here == FARMING_BLOCK_PUMPKIN_FRUIT) {
        y.block_drop = FARMING_BLOCK_PUMPKIN_FRUIT;
        y.xp = 1;
        world_set_block(w, wx, wy, wz, BLOCK_AIR);
    }
    return y;
}

int farming_harvest_was_full(const farming_yield *y) {
    return y->produce_count > 0 || y->block_drop != 0;
}
