#ifndef WORLD_OREGEN_BUFFER_H
#define WORLD_OREGEN_BUFFER_H

#include "oregen_types.h"

// caller-owned cell buffer. the blob builders emit voxels here without
// caring about chunks; the driver drains it into the world afterward. same
// no-malloc-no-ownership deal as gen2_place_buf, just our cell type.

typedef struct {
    oregen_cell *items;
    int count;
    int cap;
    int dropped;   // how many adds we had to throw away because we ran full
} oregen_buf;

void oregen_buf_init(oregen_buf *b, oregen_cell *storage, int cap);

// add one cell. returns 1 on success, 0 if full (and bumps dropped).
int  oregen_buf_add(oregen_buf *b, int x, int y, int z, block_id id);

// true if the buffer cant take any more.
int  oregen_buf_full(const oregen_buf *b);

// clear without touching storage.
void oregen_buf_reset(oregen_buf *b);

#endif
