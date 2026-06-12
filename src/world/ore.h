#ifndef WORLD_ORE_H
#define WORLD_ORE_H

#include "chunk.h"

// for now we only have stone, dirt, etc in the block registry.
// this module sprinkles "ore veins" of cobble (placeholder) based on noise.
// TODO: add actual ore block types to block.c and wire them up here

void ore_sprinkle(chunk *c, unsigned seed);

#endif
