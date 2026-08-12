#ifndef PLAYER_FISHING_WATER_H
#define PLAYER_FISHING_WATER_H

#include "fishing_types.h"
#include "../../world/world.h"

// figuring out what kind of water a bobber is sitting in. open water (a clear
// surface with sky above) fishes best; if it's penned in by blocks or roofed
// over, bites come slower or not at all. keeps the bobber sim dumb.

// is the block at this world cell water.
int  fishing_water_is_water(world *w, int wx, int wy, int wz);

// snap a y down to the surface of the water column under p. returns the y of
// the topmost water cell, or INT_MIN-ish via the out flag if none found.
int  fishing_water_surface_y(world *w, int wx, int wz, int *found);

// classify the 3x3 surface patch centred on (wx,wz) at surface level. used the
// moment the bobber lands to decide how generous the bite timer should be.
fishing_water_kind fishing_water_classify(world *w, int wx, int wy, int wz);

// a coarse 0..1 quality score for a patch, folding in open-ness and sky access.
// the bite timer scales against this so good spots pay off.
float fishing_water_quality(world *w, int wx, int wy, int wz);

#endif
