#ifndef WORLD_RIVERS_H
#define WORLD_RIVERS_H

#include "chunk.h"

// carves rivers into chunks using 2d noise bands. dumps water blocks in.
void rivers_carve(chunk *c, unsigned seed);

#endif
