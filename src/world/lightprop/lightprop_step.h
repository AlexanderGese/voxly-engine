#ifndef WORLD_LIGHTPROP_STEP_H
#define WORLD_LIGHTPROP_STEP_H

#include "lightprop_types.h"

// the per-neighbour math, shared by block and sky propagation. pulling it out
// keeps the bfs loops honest about doing the exact same arithmetic both ways.

// what light level should propagate from a cell of `level` INTO a neighbour
// holding block `nid`? accounts for the base step cost (1) plus medium
// attenuation. returns 0 if nothing should pass (opaque, or it'd underflow).
uint8_t lp_step_falloff(uint8_t level, block_id nid);

// sky light has one special case: light travelling straight DOWN through fully
// transparent space does not attenuate (the classic minecraft "sunlight column"
// rule). this answers whether the move from cell at `level` going in direction
// `dir` (index into LP_DX/DY/DZ) is such a freebie.
int lp_step_is_sky_drop(lp_channel ch, int dir, uint8_t level, block_id nid);

// combined: the resulting level a neighbour would get, honouring the sky drop.
uint8_t lp_step_propagate(lp_channel ch, int dir, uint8_t level, block_id nid);

#endif
