#ifndef WORLD_HEIGHTMAP_FIELD_H
#define WORLD_HEIGHTMAP_FIELD_H

#include "heightmap_params.h"
#include "heightmap_column.h"

// a chunk-footprint patch of resolved heights, plus a one-cell skirt so the
// mesher / surface rules can look at the neighbour column without resolving it
// twice or reaching into the next chunk. row-major, includes the pad. exactly
// the same shape idea as the erosion field next door, on purpose, so the two
// can hand data back and forth without reformatting.

#define HEIGHTMAP_PAD     1
#define HEIGHTMAP_DIM     (16 + 2 * HEIGHTMAP_PAD)   // CHUNK_SIZE_X + skirt
#define HEIGHTMAP_CELLS   (HEIGHTMAP_DIM * HEIGHTMAP_DIM)

typedef struct {
    int   ox, oz;                       // world coord of cell (PAD,PAD)
    int   surface[HEIGHTMAP_CELLS];     // top y per column
    float steepness[HEIGHTMAP_CELLS];   // mirror of column steepness
    uint8_t underwater[HEIGHTMAP_CELLS];
} heightmap_field;

// flat index into the padded grid. local coords run from -PAD..15+PAD.
static inline int heightmap_field_idx(int lx, int lz) {
    return (lz + HEIGHTMAP_PAD) * HEIGHTMAP_DIM + (lx + HEIGHTMAP_PAD);
}

// resolve a whole tile rooted at chunk-origin world coords (cx_world, cz_world
// = chunk origin in blocks). fills every padded cell.
void heightmap_field_build(heightmap_field *f, const heightmap_params *p,
                           int cx_world, int cz_world);

// read helpers, clamped to the padded range so callers cant walk off.
int   heightmap_field_at(const heightmap_field *f, int lx, int lz);
float heightmap_field_steep_at(const heightmap_field *f, int lx, int lz);

// min/max surface across the tile interior, handy for early-out when a chunk
// is entirely below or above the terrain band.
void  heightmap_field_minmax(const heightmap_field *f, int *out_min, int *out_max);

#endif
