#ifndef WORLD_FARMING_PLANT_H
#define WORLD_FARMING_PLANT_H

#include "../world.h"
#include "farming_types.h"

// planting: turning a seed item into a stage-0 crop on a hydrated farmland tile.
// also the tilling op that makes the farmland in the first place, since the two
// go hand in hand.

// till the block at (wx,wy,wz) into farmland. succeeds only if the block is
// dirt/grass and the block above is air (cant till under something). returns 1
// on success. the field manager is expected to register a tile afterward.
int farming_plant_till(world *w, int wx, int wy, int wz);

// plant a crop of `kind` at (wx,wy,wz). validates plantability, writes the
// stage-0 crop block, and fills `out` with the initialised crop record. returns
// 1 on success, 0 if the spot was illegal.
int farming_plant_crop(world *w, int wx, int wy, int wz,
                       farming_crop_kind kind, uint32_t tick,
                       farming_crop *out);

// uproot a crop without harvesting (breaking an immature plant). clears the
// block and returns the seeds you get back: immature wheat/beet give 1 seed,
// carrots/potatoes give 1 of themselves, stems give 1 seed. 0 if nothing there.
int farming_plant_uproot(world *w, int wx, int wy, int wz,
                         farming_crop_kind kind);

#endif
