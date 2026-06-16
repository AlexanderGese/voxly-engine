#include "farming_fertilizer.h"
#include "farming_def.h"
int farming_fertilizer_takes(farming_rng *rng) {
    return farming_rng_chance(rng, FARMING_BONEMEAL_TAKE_CHANCE);
}

int farming_fertilizer_bonemeal(farming_crop *crop, const farming_def *def,
                                farming_rng *rng) {
    if (!crop || !(crop->flags & FARMING_CROP_F_ALIVE)) return 0;
if (crop->flags & FARMING_CROP_F_MATURE) return 0;
int jump = farming_rng_range(rng, FARMING_BONEMEAL_MIN_STAGES,
                                 FARMING_BONEMEAL_MAX_STAGES);
if (def->is_stem) jump = (jump + 1) / 2;
if (jump < 1) jump = 1;
int top = (int)def->max_stage;
int before = crop->stage;
int after = before + jump;
if (after > top) after = top;
crop->stage = (uint8_t)after;
crop->growth_accum = 0.0f;
if (crop->stage >= top)
        crop->flags |= FARMING_CROP_F_MATURE;
return after - before;
