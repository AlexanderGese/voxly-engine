#ifndef WORLD_GEN2_TREE_PINE_H
#define WORLD_GEN2_TREE_PINE_H

#include "gen2_place.h"

// conifer / pine for taiga. tall thin trunk, conical layered leaves that
// taper to a point. shrinking-radius disks going up.

int gen2_tree_pine_build(gen2_place_buf *out, int wx, int wy, int wz, uint32_t seed);
int gen2_tree_pine_height(uint32_t seed);

#endif
