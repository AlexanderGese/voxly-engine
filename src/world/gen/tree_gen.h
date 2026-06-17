#ifndef WORLD_GEN_TREE_GEN_H
#define WORLD_GEN_TREE_GEN_H

#include "../chunk.h"
#include "../../math/rng.h"

// different tree types by biome.

typedef enum {
    TREE_OAK = 0,
    TREE_BIRCH,
    TREE_SPRUCE,
    TREE_JUNGLE,
    TREE_ACACIA,
} tree_type;

void tree_gen_place(chunk *c, int lx, int ly, int lz, tree_type type, rng *r);

#endif
