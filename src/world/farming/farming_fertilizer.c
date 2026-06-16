#include "farming_fertilizer.h"
#include "farming_def.h"

int farming_fertilizer_takes(farming_rng *rng) {
    return farming_rng_chance(rng, FARMING_BONEMEAL_TAKE_CHANCE);
}

int farming_fertilizer_bonemeal(farming_crop *crop, const farming_def *def,
                                farming_rng *rng) {
    if (!crop || !(crop->flags & FARMING_CROP_F_ALIVE)) return 0;
    if (crop->flags & FARMING_CROP_F_MATURE) return 0;

    // roll the raw jump.
    int jump = farming_rng_range(rng, FARMING_BONEMEAL_MIN_STAGES,
                                 FARMING_BONEMEAL_MAX_STAGES);

    // stems resist bonemeal: halve the jump (rounded up), min 1. keeps instant
    // pumpkins from being trivial.
    if (def->is_stem) jump = (jump + 1) / 2;
    if (jump < 1) jump = 1;

    int top = (int)def->max_stage;
    int before = crop->stage;
    int after = before + jump;
    if (after > top) after = top;

    crop->stage = (uint8_t)after;
    // bonemealing resets the accumulator; you bought the stage outright.
    crop->growth_accum = 0.0f;

    if (crop->stage >= top)
        crop->flags |= FARMING_CROP_F_MATURE;

    return after - before;
}

int farming_fertilizer_compost(farming_tile *tile, int amount) {
    if (!tile) return 0;
    if (amount < 0) amount = 0;

    int h = tile->hydration + amount;
    if (h > FARMING_HYDRATION_MAX) h = FARMING_HYDRATION_MAX;
    tile->hydration = (uint8_t)h;
    // a fresh top-up also resets the dry clock, same as watering would.
    tile->dry_timer = 0.0f;
    return tile->hydration;
}
