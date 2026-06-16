#ifndef WORLD_FARMING_HYDRATION_H
#define WORLD_FARMING_HYDRATION_H

#include "../world.h"
#include "farming_types.h"

// hydration model. a farmland tile is "near water" if any water block sits
// within FARMING_WATER_RADIUS on xz and within +-1 on y of the tile. near
// water it tops up toward FARMING_HYDRATION_MAX; away from water it dries on a
// timer. dry farmland eventually wants to revert to dirt (handled by the tile
// state machine, this module only moves the number).

// scan the world for water around (wx,wy,wz). returns 1 if hydrating water was
// found in range. cheap chebyshev box scan, water is rare enough to not matter.
int farming_hydration_water_near(world *w, int wx, int wy, int wz);

// also count rain as a water source if the sky is open above the tile. we dont
// have a weather handle here, so the caller passes whether its currently
// raining; we just check the column is exposed to sky.
int farming_hydration_sky_open(world *w, int wx, int wy, int wz);

// advance a tile's hydration for `dt` seconds given whether water is near and
// whether rain is reaching it. mutates tile->hydration and tile->dry_timer.
void farming_hydration_step(farming_tile *tile, int water_near, int rained,
                            float dt);

// 0..1 wetness used by the growth rate scaler. derived from the 0..7 counter
// with a soft floor so bone-dry land still creeps along, just slowly.
float farming_hydration_factor(const farming_tile *tile);

#endif
