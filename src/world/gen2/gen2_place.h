#ifndef WORLD_GEN2_PLACE_H
#define WORLD_GEN2_PLACE_H

#include <stdint.h>
#include "../block.h"

// a tiny generic "block placement" buffer so structure/tree builders can
// emit voxels without knowing about chunks. the worldgen driver can then
// stamp these into the world however it likes. keeps gen2 link-clean.

typedef struct {
    int x, y, z;     // world coords
    block_id id;
} gen2_placement;

typedef struct {
    gen2_placement *items;
    int count;
    int cap;
} gen2_place_buf;

// init with a caller-owned array. no malloc, no ownership games.
void gen2_place_init(gen2_place_buf *b, gen2_placement *storage, int cap);

// add one placement. silently drops if full (returns 0), 1 on success.
int gen2_place_add(gen2_place_buf *b, int x, int y, int z, block_id id);

// clear without touching storage.
void gen2_place_reset(gen2_place_buf *b);

#endif
