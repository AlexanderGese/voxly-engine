#ifndef ENTITY_SPAWN_MSPAWN_BUDGET_H
#define ENTITY_SPAWN_MSPAWN_BUDGET_H
#include "mspawn_types.h"
#include "mspawn_density.h"
// dynamic cap shaping. the raw ceilings in mspawn_density are the *baseline*;
// this module nudges them per pass from a few cheap signals so the population
// breathes instead of sitting at a flat number:
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
void mspawn_budget_defaults(mspawn_budget_cfg *cfg);
int  mspawn_budget_apply(mspawn_density *d, const mspawn_budget_cfg *cfg,
                         float day_hour);
float mspawn_budget_night_factor(float day_hour);
float mspawn_budget_moon_factor(int moon_phase);
#endif
