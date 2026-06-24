#ifndef WORLD_RIVERS_FIELD_H
#define WORLD_RIVERS_FIELD_H

#include <stdint.h>
#include "rivers_types.h"

// the scratch the whole carver scribbles on. it's a stack of parallel 2d grids,
// one cell per (x,z) column over a padded chunk footprint. allocated once,
// reset per region. each pass owns a couple of the planes:
//
// surface   land height per column (filled by caller from the heightmap)
// filled    height after depression-fill raised the pits (lake pass)
// accum     flow accumulation, units of rain collected (flow pass)
// dir       d8 downhill direction (flow pass)
// wet       what the cell ended up as, a rivers_wet (trace + fill passes)
// water_y   resolved water surface height where wet != DRY
//
// keeping them as separate planes instead of an array-of-structs was a
// deliberate call: every pass sweeps one or two planes linearly, so this is
// cache-friendlier and the reset memsets are trivial.

typedef struct {
    int   base_x;     // world x of cell (RIVERS_PAD, RIVERS_PAD)
    int   base_z;
    int   chunk_cx;
    int   chunk_cz;
} rivers_origin;

typedef struct {
    rivers_origin origin;

    int     surface[RIVERS_CELLS];   // input land height
    int     filled [RIVERS_CELLS];   // post depression-fill height
    float   accum  [RIVERS_CELLS];   // flow accumulation
    uint8_t dir    [RIVERS_CELLS];   // rivers_dir
    uint8_t wet    [RIVERS_CELLS];   // rivers_wet
    int     water_y[RIVERS_CELLS];   // water surface y where wet

    int     dirty;                   // got any wet cells at all
} rivers_field;

rivers_field *rivers_field_create(void);
void          rivers_field_destroy(rivers_field *f);

// wipe planes and stamp the region origin. surface is left for the caller to
// fill via rivers_field_set_surface (it comes from the terrain heightmap).
void rivers_field_reset(rivers_field *f, rivers_origin origin);

// flat index + bounds. caller usually knows it's in range; the checked variant
// is for the neighbour sweeps that walk off the edge.
int  rivers_field_idx(int x, int z);
int  rivers_field_in_bounds(int x, int z);

// surface height accessors (pad-inclusive cell coords).
void rivers_field_set_surface(rivers_field *f, int x, int z, int y);
int  rivers_field_surface(const rivers_field *f, int x, int z);

// "is this cell on the 1-wide outer ring". flow that points off the ring is
// treated as draining out of the region.
int  rivers_field_on_edge(int x, int z);

// map a cell coord to a world column coord and back.
void rivers_field_to_world(const rivers_field *f, int x, int z,
                           int *wx, int *wz);
int  rivers_field_from_world(const rivers_field *f, int wx, int wz,
                             int *x, int *z);

// count cells whose wet plane equals a given state. for stats and tests.
int  rivers_field_count_wet(const rivers_field *f, uint8_t state);

#endif
