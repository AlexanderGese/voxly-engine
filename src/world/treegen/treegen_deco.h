#ifndef WORLD_TREEGEN_DECO_H
#define WORLD_TREEGEN_DECO_H

#include <stdint.h>
#include "treegen_types.h"
#include "treegen_grass.h"
#include "../chunk.h"

// the decoration pass: the one entry point the worldgen driver actually calls.
// given a generated chunk and a height/surface lookup, it places trees, bushes
// and ground cover. it owns a reusable voxel buffer internally so a worldgen
// thread can decorate chunk after chunk without churning allocations.
//
// trees are placed on a coarse jitter-grid in *world* space, not per-chunk, so a
// tree whose trunk sits in the next chunk still drops its canopy into this one.
// that's the whole reason this is buffer-based: we grow the plant once at its
// world anchor and stamp only the cells that fall inside the chunk we're filling.

// surface query the driver supplies: top solid y and block id at a world column.
// returns 1 if the column has a valid surface, 0 to skip (water, void, etc).
typedef int (*treegen_surface_fn)(void *user, int wx, int wz,
                                  int *out_y, block_id *out_surface);

typedef struct {
    uint32_t seed;              // world seed, mixed per-plant internally
    int   tree_grid;            // tree candidate spacing in blocks (jitter grid)
    float tree_chance;          // per grid cell, odds a tree actually spawns
    float bush_chance;          // per grid cell, odds of a bush instead/also
    treegen_scatter scatter;    // ground cover config
    int   place_cover;          // 0 disables grass/flower pass
} treegen_deco_config;

treegen_deco_config treegen_deco_config_default(uint32_t seed);

// reusable per-thread context. init once, decorate many chunks, free at the end.
typedef struct {
    struct treegen_buffer *buf;   // owned, opaque here to keep the header light
    treegen_deco_config cfg;
} treegen_deco;

void treegen_deco_init(treegen_deco *d, const treegen_deco_config *cfg);
void treegen_deco_free(treegen_deco *d);

// decorate one chunk in place. `surf`/`user` resolve world surface heights so we
// can root plants on the ground and clip canopies to the chunk. returns the
// number of blocks written.
int  treegen_deco_chunk(treegen_deco *d, chunk *c,
                        treegen_surface_fn surf, void *user);

#endif
