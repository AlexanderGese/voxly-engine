#ifndef WORLD_FARMING_FERTILIZER_H
#define WORLD_FARMING_FERTILIZER_H

#include "farming_types.h"
#include "farming_rng.h"
#include "farming_def.h"

// fertilizer / bonemeal. the impatient gardener's tool: a pinch shoves a crop
// forward a random number of stages instead of waiting on growth ticks. stems
// get a smaller nudge because instant melons feel cheap. there's also a slow
// "compost" that just tops a tile's hydration back up.

// how many stages a single application can jump, before per-species scaling.
#define FARMING_BONEMEAL_MIN_STAGES   2
#define FARMING_BONEMEAL_MAX_STAGES   5

// apply bonemeal to a crop. rolls a stage jump, clamps to the species max, and
// flips FARMING_CROP_F_MATURE if it reaches the top. returns the number of
// stages actually advanced (0 if already mature or a null crop). does NOT touch
// the world block; the field manager syncs the block from crop->stage after.
int farming_fertilizer_bonemeal(farming_crop *crop, const farming_def *def,
                                farming_rng *rng);

// the chance bonemeal "takes" at all on a given application. vanilla-ish: not
// every pinch advances, which makes a stack feel like a gamble.
#define FARMING_BONEMEAL_TAKE_CHANCE   0.45f

// roll whether an application takes effect this time.
int farming_fertilizer_takes(farming_rng *rng);

// compost a tile: bump hydration toward max by `amount` points (clamped). a
// cheap way to rescue a drying tile without water nearby. returns the new
// hydration value.
int farming_fertilizer_compost(farming_tile *tile, int amount);

#endif
