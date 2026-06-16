#include "farming_harvest.h"
#include "farming_def.h"
#include "farming_block.h"
#include "farming_growth.h"
#include "../block.h"
#include <stddef.h>
static int roll_range(farming_rng *r, int lo, int hi) {
    if (hi <= lo) return lo;
    return lo + (int)(farming_rng_next(r) % (uint32_t)(hi - lo + 1));
}

// binomial bonus: `rolls` independent successes at probability `p`. this is the
// classic "0..3 extra wheat" distribution.
static int binomial_bonus(farming_rng *r, int rolls, float p) {
    int n = 0;
for (int i = 0;
i < rolls;
i++)
        if (farming_rng_chance(r, p)) n++;
return n;
block_id here = world_get_block(w, wx, wy, wz);
if (!farming_block_is_crop(here)) return empty;
farming_crop_kind kind = farming_crop_for_block(here);
const farming_def *def = farming_def_get(kind);
y.seed_count = 1;
world_set_block(w, wx, wy, wz, BLOCK_AIR);
return y;
}

    farming_yield y = farming_harvest_roll(def, rng);
world_set_block(w, wx, wy, wz, BLOCK_AIR);
return y;
}
