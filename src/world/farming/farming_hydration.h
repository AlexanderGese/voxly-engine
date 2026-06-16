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
int farming_hydration_sky_open(world *w, int wx, int wy, int wz);
void farming_hydration_step(farming_tile *tile, int water_near, int rained,
                            float dt);
float farming_hydration_factor(const farming_tile *tile);
#endif
