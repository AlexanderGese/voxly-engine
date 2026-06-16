#ifndef WORLD_FARMING_DEF_H
#define WORLD_FARMING_DEF_H

#include "farming_types.h"

// per-species static definition. the growth model is: each crop accumulates
// "growth points" per tick scaled by environment, and crosses a stage every
// `points_per_stage`. different species just dial the curve and the yields.

typedef struct {
    const char *name;
    uint8_t max_stage;        // inclusive top stage (mature)
    uint8_t is_stem;          // stems fruit a neighbor instead of giving produce
    float   points_per_stage; // accumulated growth needed to advance one stage
    float   base_rate;        // growth points per growth tick at full hydration

    // yields rolled at harvest. produce is the food drop; the binomial below
    // gives the classic "0..n bonus" spread minecraft-style.
    int   produce_min, produce_max;
    int   seed_min, seed_max;
    float bonus_chance;       // per extra-roll success probability
    int   bonus_rolls;        // number of extra binomial rolls
    int   xp;                 // xp on a mature harvest
} farming_def;

// fetch the def for a species. always returns a valid pointer; FARMING_CROP_NONE
// and out-of-range map to a harmless "null crop" entry so callers neednt guard.
const farming_def *farming_def_get(farming_crop_kind kind);

// convenience: the mature stage for a species.
int farming_def_max_stage(farming_crop_kind kind);

// is this species a fruit-spawning stem (melon/pumpkin)?
int farming_def_is_stem(farming_crop_kind kind);

#endif
