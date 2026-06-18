#ifndef WORLD_GEN2_STRUCTURE_WELL_H
#define WORLD_GEN2_STRUCTURE_WELL_H

#include "gen2_place.h"

// little stone-brick desert well. 4x4 rim, water in the middle, short
// posts and a flat cap. emits into a placement buffer.

int gen2_well_should_place(int chunk_x, int chunk_z, uint32_t seed);

// floor of the well at (wx,wy,wz) is the rim level. digs water down 2.
int gen2_well_build(gen2_place_buf *out, int wx, int wy, int wz);

#endif
