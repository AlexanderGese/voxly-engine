#ifndef WORLD_WORLD_H
#define WORLD_WORLD_H

#include "chunk.h"
#include "../math/vec3.h"

// the world owns all loaded chunks. simple linked list for now.
// TODO: swap to hashmap when this gets slow

typedef struct world_node {
    chunk *c;
    struct world_node *next;
} world_node;

typedef struct {
    world_node *head;
    int         count;
    unsigned    seed;
} world;

world *world_create(unsigned seed);
void   world_destroy(world *w);

// chunk lookup
chunk *world_get_chunk(world *w, int cx, int cz);
chunk *world_get_or_create(world *w, int cx, int cz);
void   world_remove_chunk(world *w, int cx, int cz);

// world-space block access. auto finds the containing chunk.
block_id world_get_block(world *w, int wx, int wy, int wz);
void     world_set_block(world *w, int wx, int wy, int wz, block_id id);

// light access, world space
uint8_t  world_get_blocklight(world *w, int wx, int wy, int wz);
uint8_t  world_get_sunlight(world *w, int wx, int wy, int wz);

// call after chunks change. updates nx/px/nz/pz pointers.
void world_update_neighbors(world *w);

// stream chunks in/out based on player position. called every frame.
void world_stream(world *w, vec3 player_pos, int load_dist, int unload_dist);

// iterator helper for rendering
typedef void (*world_visit_fn)(chunk *c, void *user);
void world_visit(world *w, world_visit_fn fn, void *user);

// coord helpers
void  world_to_chunk(int wx, int wz, int *cx, int *cz);
void  world_to_local(int wx, int wz, int *lx, int *lz);

#endif
