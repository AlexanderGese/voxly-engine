#ifndef WORLD_GEN2_TREE_OAK_H
#define WORLD_GEN2_TREE_OAK_H

#include "gen2_place.h"

// classic broadleaf oak. straight trunk, roundish leaf blob on top.
// emits into a placement buffer at world coords. base = block under trunk.

// returns number of blocks emitted. seed makes height/shape vary.
int gen2_tree_oak_build(gen2_place_buf *out, int wx, int wy, int wz, uint32_t seed);

// just the trunk height, exposed so callers can reserve space.
int gen2_tree_oak_height(uint32_t seed);

#endif
