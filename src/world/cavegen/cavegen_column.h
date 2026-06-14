#ifndef WORLD_CAVEGEN_COLUMN_H
#define WORLD_CAVEGEN_COLUMN_H

#include "cavegen_grid.h"
#include "../chunk.h"

// fills the grid's per-column heightmap. the automata uses this to keep a solid
// crust under the surface so caves never breach into daylight (well, hardly
// ever — a worm can still poke a cliff face, which honestly looks great).

// scan a chunk top-down to find the highest solid block per column and write it
// into the grid heightmap for the centre footprint cells.
void cavegen_column_scan_chunk(cavegen_grid *g, const chunk *c);

// fill the pad columns by copying from the chunk's edge columns. crude but the
// pad only exists for neighbour context and the heightmap there just needs to
// be "roughly right" so the crust check doesnt do anything stupid at the seam.
void cavegen_column_fill_pad(cavegen_grid *g);

// find the topmost solid y in one column of a chunk, or -1 if the whole column
// is air (can happen over oceans before water fill).
int  cavegen_column_surface_y(const chunk *c, int lx, int lz);

#endif
