#ifndef WORLD_FARMING_STEM_H
#define WORLD_FARMING_STEM_H

#include "../world.h"
#include "farming_types.h"
#include "farming_rng.h"

// melon/pumpkin stem behaviour. a stem grows like any other crop until mature,
// then instead of being harvested it tries, on each subsequent growth tick, to
// spawn a fruit block in a free cardinal cell. once fruited it idles until the
// fruit is removed, then it can fruit again.

// try to spawn a fruit for a mature stem at (wx,wy,wz). picks a free cardinal
// spot (see farming_query_free_fruit_spots), rolls the chance, and on success
// places the fruit block and sets FARMING_CROP_F_FRUITED. returns 1 if a fruit
// was spawned this call.
int farming_stem_try_fruit(world *w, farming_crop *stem, farming_rng *rng);

// re-check whether a fruited stem still has its fruit. if the adjacent fruit was
// harvested/broken, clears FARMING_CROP_F_FRUITED so the stem can fruit again.
// returns 1 if the stem became fruitable again this call.
int farming_stem_recheck(world *w, farming_crop *stem);

// chance per growth tick that a mature, unfruited stem spawns a fruit. low, so
// fruit feels earned, not spammed.
#define FARMING_STEM_FRUIT_CHANCE   0.18f

#endif
