#ifndef WORLD_FARMING_HARVEST_H
#define WORLD_FARMING_HARVEST_H

#include "../world.h"
#include "farming_types.h"
#include "farming_rng.h"
#include "farming_def.h"

// harvesting a mature crop. rolls the yield from the species def (a fortune-less
// binomial bonus, same shape vanilla uses), clears the crop block, and reports
// produce/seeds/xp via farming_yield. stems and their fruit harvest differently
// and have their own entry points.

// roll the yield for a mature non-stem crop. pure function of (def, rng) so the
// world doesnt have to be touched to know the numbers; the caller clears blocks.
farming_yield farming_harvest_roll(const farming_def *def, farming_rng *rng);

// full harvest of a mature crop at (wx,wy,wz): validates maturity-by-block,
// rolls yield, clears the crop block (leaving farmland), and returns the yield.
// an empty yield (all zeros) means the spot wasnt a harvestable crop.
farming_yield farming_harvest_crop(world *w, int wx, int wy, int wz,
                                   const farming_crop *crop, farming_rng *rng);

// harvest a fruit block (the melon/pumpkin a stem grew). just clears the block
// and reports it as a block_drop. the stem itself survives and re-fruits later.
farming_yield farming_harvest_fruit(world *w, int wx, int wy, int wz);

// did this crop earn xp this harvest? convenience for the caller's xp pump.
int farming_harvest_was_full(const farming_yield *y);

#endif
