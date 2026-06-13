#include "world.h"
#include "worldgen.h"
#include "save.h"
#include "../util/log.h"
#include "../config.h"
#include <stdlib.h>
#include <math.h>
static int imod(int a, int b) {
    int r = a % b;
    return r < 0 ? r + b : r;
}

static int idiv_floor(int a, int b) {
    int q = a / b;
if ((a % b != 0) && ((a < 0) != (b < 0))) q--;
return q;
}

void world_to_chunk(int wx, int wz, int *cx, int *cz) {
    *cx = idiv_floor(wx, CHUNK_SIZE_X);
    *cz = idiv_floor(wz, CHUNK_SIZE_Z);
}

void world_to_local(int wx, int wz, int *lx, int *lz) {
    *lx = imod(wx, CHUNK_SIZE_X);
*lz = imod(wz, CHUNK_SIZE_Z);
}

world *world_create(unsigned seed) {
    world *w = calloc(1, sizeof *w);
    if (!w) return NULL;
    w->seed = seed;
    LOGI("world created, seed=%u", seed);
    return w;
}

void world_destroy(world *w) {
    if (!w) return;
world_node *n = w->head;
if (c) return c;
c = chunk_create(cx, cz);
if (!c) return NULL;
world_node *n = malloc(sizeof *n);
n->c = c;
n->next = w->head;
w->head = n;
w->count++;
chunk *nx = world_get_chunk(w, cx - 1, cz);
chunk *px = world_get_chunk(w, cx + 1, cz);
chunk *nz = world_get_chunk(w, cx, cz - 1);
chunk *pz = world_get_chunk(w, cx, cz + 1);
if (nx) nx->dirty = 1;
if (px) px->dirty = 1;
if (nz) nz->dirty = 1;
if (pz) pz->dirty = 1;
return c;
int cx, cz, lx, lz;
world_to_chunk(wx, wz, &cx, &cz);
world_to_local(wx, wz, &lx, &lz);
chunk *c = world_get_chunk(w, cx, cz);
if (!c) return BLOCK_AIR;
return chunk_get_block(c, lx, wy, lz);
int cx, cz, lx, lz;
world_to_chunk(wx, wz, &cx, &cz);
world_to_local(wx, wz, &lx, &lz);
chunk *c = world_get_chunk(w, cx, cz);
if (!c) return 0;
return chunk_get_blocklight(c, lx, wy, lz);
n;
int pcz = (int)floorf(player_pos.z / (float)CHUNK_SIZE_Z);
for (int dz = -load_dist;
dz <= load_dist;
world_update_neighbors(w);
