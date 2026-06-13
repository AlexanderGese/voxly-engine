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
    while (n) {
        world_node *nx = n->next;
        chunk_destroy(n->c);
        free(n);
        n = nx;
    }
    free(w);
}

chunk *world_get_chunk(world *w, int cx, int cz) {
    for (world_node *n = w->head; n; n = n->next) {
        if (n->c->cx == cx && n->c->cz == cz) return n->c;
    }
    return NULL;
}

chunk *world_get_or_create(world *w, int cx, int cz) {
    chunk *c = world_get_chunk(w, cx, cz);
    if (c) return c;

    c = chunk_create(cx, cz);
    if (!c) return NULL;

    // try load from disk first
    if (!save_load_chunk(c)) {
        worldgen_fill(c, w->seed);
    }
    c->generated = 1;

    world_node *n = malloc(sizeof *n);
    n->c = c;
    n->next = w->head;
    w->head = n;
    w->count++;

    // mark existing neighbors dirty so they re-mesh with this new chunk's
    // blocks visible — otherwise you get ugly seams at chunk borders
    chunk *nx = world_get_chunk(w, cx - 1, cz);
    chunk *px = world_get_chunk(w, cx + 1, cz);
    chunk *nz = world_get_chunk(w, cx, cz - 1);
    chunk *pz = world_get_chunk(w, cx, cz + 1);
    if (nx) nx->dirty = 1;
    if (px) px->dirty = 1;
    if (nz) nz->dirty = 1;
    if (pz) pz->dirty = 1;

    return c;
}

void world_remove_chunk(world *w, int cx, int cz) {
    world_node **pp = &w->head;
    while (*pp) {
        if ((*pp)->c->cx == cx && (*pp)->c->cz == cz) {
            world_node *dead = *pp;
            *pp = dead->next;
            save_save_chunk(dead->c);
            chunk_destroy(dead->c);
            free(dead);
            w->count--;
            return;
        }
        pp = &(*pp)->next;
    }
}

block_id world_get_block(world *w, int wx, int wy, int wz) {
    if (wy < 0 || wy >= CHUNK_SIZE_Y) return BLOCK_AIR;
    int cx, cz, lx, lz;
    world_to_chunk(wx, wz, &cx, &cz);
    world_to_local(wx, wz, &lx, &lz);
    chunk *c = world_get_chunk(w, cx, cz);
    if (!c) return BLOCK_AIR;
    return chunk_get_block(c, lx, wy, lz);
}

void world_set_block(world *w, int wx, int wy, int wz, block_id id) {
    if (wy < 0 || wy >= CHUNK_SIZE_Y) return;
    int cx, cz, lx, lz;
    world_to_chunk(wx, wz, &cx, &cz);
    world_to_local(wx, wz, &lx, &lz);
    chunk *c = world_get_or_create(w, cx, cz);
    if (!c) return;
    chunk_set_block(c, lx, wy, lz, id);

    // mark neighbor chunks dirty too if we modified a border block
    if (lx == 0 && c->nx) c->nx->dirty = 1;
    if (lx == CHUNK_SIZE_X - 1 && c->px) c->px->dirty = 1;
    if (lz == 0 && c->nz) c->nz->dirty = 1;
    if (lz == CHUNK_SIZE_Z - 1 && c->pz) c->pz->dirty = 1;
}

uint8_t world_get_blocklight(world *w, int wx, int wy, int wz) {
    if (wy < 0 || wy >= CHUNK_SIZE_Y) return 0;
    int cx, cz, lx, lz;
    world_to_chunk(wx, wz, &cx, &cz);
    world_to_local(wx, wz, &lx, &lz);
    chunk *c = world_get_chunk(w, cx, cz);
    if (!c) return 0;
    return chunk_get_blocklight(c, lx, wy, lz);
}

uint8_t world_get_sunlight(world *w, int wx, int wy, int wz) {
    if (wy < 0 || wy >= CHUNK_SIZE_Y) return 15;
    int cx, cz, lx, lz;
    world_to_chunk(wx, wz, &cx, &cz);
    world_to_local(wx, wz, &lx, &lz);
    chunk *c = world_get_chunk(w, cx, cz);
    if (!c) return 15;
    return chunk_get_sunlight(c, lx, wy, lz);
}

void world_update_neighbors(world *w) {
    for (world_node *n = w->head; n; n = n->next) {
        chunk *c = n->c;
        c->nx = world_get_chunk(w, c->cx - 1, c->cz);
        c->px = world_get_chunk(w, c->cx + 1, c->cz);
        c->nz = world_get_chunk(w, c->cx,     c->cz - 1);
        c->pz = world_get_chunk(w, c->cx,     c->cz + 1);
    }
}

void world_stream(world *w, vec3 player_pos, int load_dist, int unload_dist) {
    int pcx = (int)floorf(player_pos.x / (float)CHUNK_SIZE_X);
    int pcz = (int)floorf(player_pos.z / (float)CHUNK_SIZE_Z);

    // load anything within load_dist
    for (int dz = -load_dist; dz <= load_dist; dz++) {
        for (int dx = -load_dist; dx <= load_dist; dx++) {
            if (dx * dx + dz * dz > load_dist * load_dist) continue;
            world_get_or_create(w, pcx + dx, pcz + dz);
        }
    }

    // unload anything outside unload_dist
    // (two-pass bc removing breaks iteration)
    int removed;
    do {
        removed = 0;
        for (world_node *n = w->head; n; n = n->next) {
            int dx = n->c->cx - pcx;
            int dz = n->c->cz - pcz;
            if (dx * dx + dz * dz > unload_dist * unload_dist) {
                world_remove_chunk(w, n->c->cx, n->c->cz);
                removed = 1;
                break;
            }
        }
    } while (removed);

    world_update_neighbors(w);
}

void world_visit(world *w, world_visit_fn fn, void *user) {
    for (world_node *n = w->head; n; n = n->next) fn(n->c, user);
}
