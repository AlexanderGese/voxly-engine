#ifndef WORLD_GEN2_STRUCTURE_RUIN_H
#define WORLD_GEN2_STRUCTURE_RUIN_H

#include "gen2_place.h"

// crumbled stone-brick ruin. a rectangular footprint of walls with chunks
// missing, so it reads as decayed. emits into a placement buffer.

// 1 in N chunks gets a ruin. returns 1 if (chunk_x,chunk_z) should host one.
int gen2_ruin_should_place(int chunk_x, int chunk_z, uint32_t seed);

// build a ruin with its floor at (wx,wy,wz). returns blocks emitted.
int gen2_ruin_build(gen2_place_buf *out, int wx, int wy, int wz, uint32_t seed);

#endif
