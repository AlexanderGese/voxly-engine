#ifndef WORLD_CHUNK_H
#define WORLD_CHUNK_H

#include "block.h"
#include "../config.h"
#include <stdint.h>

// a chunk is 16 x 128 x 16 blocks. (cx, cz) address.
// blocks[x + z*SIZE_X + y*SIZE_X*SIZE_Z]

typedef struct chunk {
    int cx, cz;
    block_id blocks[CHUNK_VOLUME];
    uint8_t  light [CHUNK_VOLUME];   // 4 lo bits = block light, 4 hi = sunlight

    int dirty;          // needs remeshing
    int generated;      // terrain filled
    int saved;          // matches disk

    // neighbor cache (not owned). set by world_update_neighbors.
    struct chunk *nx, *px, *nz, *pz;

    // mesh buffer handles, owned by render/mesher
    unsigned int vao, vbo;
    int vertex_count;
} chunk;

chunk *chunk_create(int cx, int cz);
void   chunk_destroy(chunk *c);

// block access, local coords (0..SIZE-1)
block_id chunk_get_block(const chunk *c, int x, int y, int z);
void     chunk_set_block(chunk *c, int x, int y, int z, block_id id);

uint8_t  chunk_get_blocklight(const chunk *c, int x, int y, int z);
uint8_t  chunk_get_sunlight(const chunk *c, int x, int y, int z);
void     chunk_set_blocklight(chunk *c, int x, int y, int z, uint8_t v);
void     chunk_set_sunlight(chunk *c, int x, int y, int z, uint8_t v);

// index helper
int      chunk_idx(int x, int y, int z);

#endif
