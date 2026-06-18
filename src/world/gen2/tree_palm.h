#ifndef WORLD_GEN2_TREE_PALM_H
#define WORLD_GEN2_TREE_PALM_H

#include "gen2_place.h"

// palm for beaches/savanna edges. leaning trunk, fronds splaying out from
// the very top. no dense canopy, just arms.

int gen2_tree_palm_build(gen2_place_buf *out, int wx, int wy, int wz, uint32_t seed);
int gen2_tree_palm_height(uint32_t seed);

#endif
