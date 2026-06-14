#ifndef WORLD_COLORLIGHT_DEBUG_H
#define WORLD_COLORLIGHT_DEBUG_H

#include "colorlight_grid.h"
#include "../world.h"

// stats + sanity checks for the rgb light. none of this runs in the hot path;
// it's for the F3-style overlay and for catching flood bugs (stale glow, levels
// that jump more than 1 between neighbors, light leaking through walls).

typedef struct {
    int   lit_cells;       // cells with any channel > 0
    int   max_r, max_g, max_b;
    int   peak_any;        // brightest single channel anywhere
    int   leak_count;      // lit cells sitting inside opaque blocks (a bug)
    int   gradient_breaks; // neighbor pairs differing by > 1 level (a bug)
    float fill_ratio;      // lit_cells / CHUNK_VOLUME
} colorlight_stats;

// walk one chunk's grid and fill out stats. needs the world to check opacity
// for the leak count.
void colorlight_debug_chunk_stats(world *w, chunk *c, colorlight_stats *out);

// returns 0 and logs if the chunk's grid violates the flood invariant (every
// lit non-source cell must have a neighbor exactly one level brighter on that
// channel). returns 1 if clean. expensive, gate behind a debug key.
int colorlight_debug_verify(world *w, chunk *c);

// dump a single y-slice of a channel to the log as ascii (0-9a-f), handy when a
// flood does something weird in one plane.
void colorlight_debug_dump_slice(chunk *c, int y, int chan);

#endif
