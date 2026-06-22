#ifndef WORLD_RAVINE_FIELD_H
#define WORLD_RAVINE_FIELD_H

#include <stdint.h>
#include "ravine_types.h"

// the per-cell scratch the mask pass scribbles and the carve pass reads. one
// entry per (x,z) cell over a padded region. struct-of-arrays so each pass
// sweeps a plane linearly, which keeps the reset memsets cheap and the cache
// happy — same call rivers made:
//
// surface   original land height per cell (filled from the heightmap)
// cut       how far below surface this cell carves (0 = untouched)
// floor_y   resolved canyon floor height where cut > 0
// kind      ravine_cellkind for the cell
//
// allocated once per worker, reset per region.

typedef struct {
    int base_x;     // world x of cell (RAVINE_PAD, RAVINE_PAD)
    int base_z;
    int chunk_cx;
    int chunk_cz;
} ravine_origin;

typedef struct {
    ravine_origin origin;

    int     surface[RAVINE_CELLS];   // input land height
    int     cut    [RAVINE_CELLS];   // carve depth below surface
    int     floor_y[RAVINE_CELLS];   // resolved floor height
    uint8_t kind   [RAVINE_CELLS];   // ravine_cellkind

    int     dirty;                   // got any carved cells at all
} ravine_field;

ravine_field *ravine_field_create(void);
void          ravine_field_destroy(ravine_field *f);

// wipe planes and stamp the region origin. surface is left for the caller to
// fill from the terrain heightmap via ravine_field_set_surface.
void ravine_field_reset(ravine_field *f, ravine_origin origin);

// bounds check (pad-inclusive cell coords). the index itself comes from the
// shared ravine_cell_index in ravine_types.
int  ravine_field_in_bounds(int x, int z);

// surface accessors.
void ravine_field_set_surface(ravine_field *f, int x, int z, int y);
int  ravine_field_surface(const ravine_field *f, int x, int z);

// cell <-> world column mapping. the pad shifts everything.
void ravine_field_to_world(const ravine_field *f, int x, int z,
                           int *wx, int *wz);
int  ravine_field_from_world(const ravine_field *f, int wx, int wz,
                             int *x, int *z);

// count cells of a given kind. for stats and tests.
int  ravine_field_count_kind(const ravine_field *f, uint8_t kind);

#endif
