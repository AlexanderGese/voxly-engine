#ifndef WORLD_COLORLIGHT_DEBUG_H
#define WORLD_COLORLIGHT_DEBUG_H
#include "colorlight_grid.h"
#include "../world.h"
typedef struct {
    int   lit_cells;       // cells with any channel > 0
    int   max_r, max_g, max_b;
    int   peak_any;        // brightest single channel anywhere
    int   leak_count;      // lit cells sitting inside opaque blocks (a bug)
    int   gradient_breaks; // neighbor pairs differing by > 1 level (a bug)
    float fill_ratio;      // lit_cells / CHUNK_VOLUME
} colorlight_stats;
#endif
