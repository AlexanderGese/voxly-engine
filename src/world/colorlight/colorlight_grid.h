#ifndef WORLD_COLORLIGHT_GRID_H
#define WORLD_COLORLIGHT_GRID_H

#include <stdint.h>
#include "../chunk.h"
#include "colorlight_packed.h"

// the rgb light volume for one chunk. CHUNK_VOLUME packed words, same indexing
// as chunk.blocks / chunk.light (chunk_idx). we keep this OUTSIDE the chunk
// struct on purpose: chunk.h is shared with the scalar lighting path and i'm
// not going to bloat every chunk by 256kb of rgb when colorlight might be off.
//
// instead a grid is allocated on demand and parked in a tiny side-table keyed
// by chunk pointer (see colorlight_grid.c). lookup is a small linear probe;
// there are only ever RENDER_DISTANCE^2-ish chunks live so this is fine.

typedef struct {
    chunk *owner;                // which chunk this belongs to (key, not owned)
    colorlight_packed *cells;    // CHUNK_VOLUME entries, malloc'd
    int dirty;                   // rgb changed, mesher should resample
} colorlight_grid;

// fetch (creating if absent) the grid for a chunk. NULL only on alloc fail.
colorlight_grid *colorlight_grid_for(chunk *c);

// fetch without creating. NULL if the chunk has no rgb grid yet.
colorlight_grid *colorlight_grid_peek(chunk *c);

// drop a chunk's grid. call from the chunk teardown path so we don't leak the
// side table. safe on a chunk that never got a grid.
void colorlight_grid_release(chunk *c);

// wipe every cell to black but keep the allocation.
void colorlight_grid_clear(colorlight_grid *g);

// local-coord (0..SIZE-1) access. out of bounds reads return 0, writes no-op.
colorlight_packed colorlight_grid_get(const colorlight_grid *g, int x, int y, int z);
void              colorlight_grid_set(colorlight_grid *g, int x, int y, int z, colorlight_packed p);

// single channel convenience, used hard by the flood inner loop.
uint8_t colorlight_grid_get_chan(const colorlight_grid *g, int x, int y, int z, int chan);
void    colorlight_grid_set_chan(colorlight_grid *g, int x, int y, int z, int chan, uint8_t v);

// how many grids are currently alive. debug / leak check.
int colorlight_grid_live_count(void);

#endif
