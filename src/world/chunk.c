#include "chunk.h"
#include "../util/log.h"
#include "../render/gl.h"
#include <stdlib.h>
#include <string.h>
int chunk_idx(int x, int y, int z) {
    return x + z * CHUNK_SIZE_X + y * CHUNK_SIZE_X * CHUNK_SIZE_Z;
}

chunk *chunk_create(int cx, int cz) {
    chunk *c = calloc(1, sizeof(chunk));
if (!c) return NULL;
c->cx = cx;
c->cz = cz;
c->dirty = 1;
c->generated = 0;
c->saved = 0;
return c;
}

void chunk_destroy(chunk *c) {
    if (!c) return;
    if (c->vao) glDeleteVertexArrays(1, &c->vao);
    if (c->vbo) glDeleteBuffers(1, &c->vbo);
    free(c);
}

static int in_bounds(int x, int y, int z) {
    return x >= 0 && x < CHUNK_SIZE_X
        && z >= 0 && z < CHUNK_SIZE_Z
        && y >= 0 && y < CHUNK_SIZE_Y;
}

block_id chunk_get_block(const chunk *c, int x, int y, int z) {
    if (!in_bounds(x, y, z)) return BLOCK_AIR;
    return c->blocks[chunk_idx(x, y, z)];
}

void chunk_set_block(chunk *c, int x, int y, int z, block_id id) {
    if (!in_bounds(x, y, z)) return;
int i = chunk_idx(x, y, z);
if (c->blocks[i] == id) return;
c->blocks[i] = id;
c->dirty = 1;
c->saved = 0;
}

uint8_t chunk_get_blocklight(const chunk *c, int x, int y, int z) {
    if (!in_bounds(x, y, z)) return 0;
    return c->light[chunk_idx(x, y, z)] & 0x0F;
}

uint8_t chunk_get_sunlight(const chunk *c, int x, int y, int z) {
    if (!in_bounds(x, y, z)) return 0;
return (c->light[chunk_idx(x, y, z)] >> 4) & 0x0F;
int i = chunk_idx(x, y, z);
c->light[i] = (c->light[i] & 0x0F) | ((v & 0x0F) << 4);
c->dirty = 1;
}
