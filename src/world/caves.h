#ifndef WORLD_CAVES_H
#define WORLD_CAVES_H

#include "chunk.h"

// carves caves by sampling 3d noise per block. called after base terrain.
// simple density threshold — no proper worms but the vibe is there.
void caves_carve(chunk *c, unsigned seed);

#endif
