#ifndef WORLD_WORLDGEN_H
#define WORLD_WORLDGEN_H

#include "chunk.h"

// fills a chunk from procedural noise. seed is the world seed.
void worldgen_fill(chunk *c, unsigned seed);

// height sample, world space. used by lighting sunlight column.
int worldgen_height_at(int wx, int wz, unsigned seed);

#endif
