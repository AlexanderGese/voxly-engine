#include "farming_def.h"
#include <stddef.h>

// the table. indexed by farming_crop_kind. tuned by feel, not by spreadsheet,
// so if wheat feels slow blame me. stems carry their max_stage as the point
// at which they stop growing tall and start trying to fruit sideways.
static const farming_def DEFS[FARMING_CROP_COUNT] = {
    // NONE: the null crop. never grows, never yields.
    [FARMING_CROP_NONE] = {
        .name = "none", .max_stage = 0, .is_stem = 0,
        .points_per_stage = 1.0f, .base_rate = 0.0f,
        .produce_min = 0, .produce_max = 0, .seed_min = 0, .seed_max = 0,
        .bonus_chance = 0.0f, .bonus_rolls = 0, .xp = 0,
    },
    [FARMING_CROP_WHEAT] = {
        .name = "wheat", .max_stage = 7, .is_stem = 0,
        .points_per_stage = 6.0f, .base_rate = 1.0f,
        .produce_min = 1, .produce_max = 1, .seed_min = 0, .seed_max = 3,
        .bonus_chance = 0.57f, .bonus_rolls = 3, .xp = 2,
    },
    [FARMING_CROP_CARROT] = {
        .name = "carrot", .max_stage = 7, .is_stem = 0,
        .points_per_stage = 6.5f, .base_rate = 1.0f,
        // carrots are their own seed, so produce doubles as replant stock.
        .produce_min = 2, .produce_max = 4, .seed_min = 0, .seed_max = 0,
        .bonus_chance = 0.57f, .bonus_rolls = 3, .xp = 2,
    },
    [FARMING_CROP_POTATO] = {
        .name = "potato", .max_stage = 7, .is_stem = 0,
        .points_per_stage = 6.5f, .base_rate = 1.0f,
        .produce_min = 2, .produce_max = 4, .seed_min = 0, .seed_max = 0,
        .bonus_chance = 0.57f, .bonus_rolls = 3, .xp = 2,
    },
    [FARMING_CROP_BEETROOT] = {
        .name = "beetroot", .max_stage = 3, .is_stem = 0,
        // fewer stages but each costs more, so it ends up roughly wheat-paced.
        .points_per_stage = 11.0f, .base_rate = 1.0f,
        .produce_min = 1, .produce_max = 1, .seed_min = 0, .seed_max = 3,
        .bonus_chance = 0.40f, .bonus_rolls = 3, .xp = 1,
    },
    [FARMING_CROP_MELON_STEM] = {
        .name = "melon_stem", .max_stage = 7, .is_stem = 1,
        .points_per_stage = 7.5f, .base_rate = 0.9f,
        // stems give seeds when broken; the fruit drops itself as a block.
        .produce_min = 0, .produce_max = 0, .seed_min = 1, .seed_max = 3,
        .bonus_chance = 0.0f, .bonus_rolls = 0, .xp = 3,
    },
    [FARMING_CROP_PUMPKIN_STEM] = {
        .name = "pumpkin_stem", .max_stage = 7, .is_stem = 1,
        .points_per_stage = 7.5f, .base_rate = 0.9f,
        .produce_min = 0, .produce_max = 0, .seed_min = 1, .seed_max = 3,
        .bonus_chance = 0.0f, .bonus_rolls = 0, .xp = 3,
    },
};

const farming_def *farming_def_get(farming_crop_kind kind) {
    if (kind < 0 || kind >= FARMING_CROP_COUNT) return &DEFS[FARMING_CROP_NONE];
    return &DEFS[kind];
}

int farming_def_max_stage(farming_crop_kind kind) {
    int m = farming_def_get(kind)->max_stage;
    return m < FARMING_MAX_STAGES ? m : FARMING_MAX_STAGES - 1;
}

int farming_def_is_stem(farming_crop_kind kind) {
    return farming_def_get(kind)->is_stem;
}
