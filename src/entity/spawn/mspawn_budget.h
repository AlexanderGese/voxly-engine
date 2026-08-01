#ifndef ENTITY_SPAWN_MSPAWN_BUDGET_H
#define ENTITY_SPAWN_MSPAWN_BUDGET_H

#include "mspawn_types.h"
#include "mspawn_density.h"

// dynamic cap shaping. the raw ceilings in mspawn_density are the *baseline*;
// this module nudges them per pass from a few cheap signals so the population
// breathes instead of sitting at a flat number:
//
// - difficulty: a 0..2 scalar (peaceful/normal/hard) scales hostiles. on
// peaceful, hostiles go to zero outright.
// - moon phase: a full moon lets a few extra hostiles in, classic touch.
// - daylight ramp: hostile budget eases in around dusk and out around dawn
// rather than snapping, so you dont get a wall of mobs at 18:00 sharp.
//
// passives and ambient are mostly steady; only hostiles get the fun knobs.

typedef enum {
    MSPAWN_DIFF_PEACEFUL = 0,
    MSPAWN_DIFF_NORMAL,
    MSPAWN_DIFF_HARD,
    MSPAWN_DIFF_COUNT
} mspawn_difficulty;

typedef struct {
    mspawn_difficulty difficulty;
    int   moon_phase;     // 0..7, 0 = full moon (brightest, most mobs)
    float hostile_mult;   // extra multiplier applied on top, for events etc
} mspawn_budget_cfg;

// sane defaults: normal difficulty, half moon, no event multiplier.
void mspawn_budget_defaults(mspawn_budget_cfg *cfg);

// recompute the category caps inside `d` from the config and the current
// clock. call once per spawn cycle before attempts. returns the resolved
// hostile cap, handy for a debug readout.
int  mspawn_budget_apply(mspawn_density *d, const mspawn_budget_cfg *cfg,
                         float day_hour);

// 0..1 ramp of how "hostile-friendly" the current hour is. exposed so callers
// can drive ambient music / fog off the same curve the caps use.
float mspawn_budget_night_factor(float day_hour);

// 0..1 moon brightness from a phase index, 1.0 at full moon.
float mspawn_budget_moon_factor(int moon_phase);

#endif
