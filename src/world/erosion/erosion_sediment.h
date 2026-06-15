#ifndef WORLD_EROSION_SEDIMENT_H
#define WORLD_EROSION_SEDIMENT_H

#include "erosion_types.h"
#include "erosion_field.h"
#include "../../math/vec2.h"

// sediment bookkeeping shared by the hydraulic pass. carve and deposit are
// smeared over a little radius instead of dumped on one cell, otherwise
// droplets drill single-pixel wells and the terrain looks like swiss cheese
// (we already have one of those, see cavegen, dont need two).

// how much load a droplet can carry given its speed, water, and the slope it
// just descended. faster + steeper + more water = more capacity. clamped at
// the params floor so a droplet on flat ground still nudges sediment along.
float erosion_carry_capacity(const erosion_params *p,
                             float slope, float speed, float water);

// drop `amount` of sediment onto the field around p, bilinear weighted to the
// four surrounding cells. returns the amount actually placed (== amount, but
// returned for the stats tally). also clears it from droplet load by caller.
float erosion_deposit(erosion_field *f, vec2 p, float amount);

// carve `amount` of height from the field in a soft disc of `radius` cells
// around p. respects per-cell hardness: hard rock gives up less. returns the
// total height actually removed (may be < amount over very hard rock).
float erosion_carve(erosion_field *f, vec2 p, float amount, float radius);

// fold standing loose sediment back into height once it falls under the
// settle threshold. anything still moving is left for the next pass. returns
// the number of cells that settled this sweep.
int erosion_settle(erosion_field *f, const erosion_params *p);

#endif
