#ifndef WORLD_HEIGHTMAP_FILL_H
#define WORLD_HEIGHTMAP_FILL_H

#include <stdint.h>
#include "heightmap_params.h"
#include "heightmap_column.h"

// turns a resolved column into the actual vertical block stack. this is the
// only file in the subsystem that decides what a block *is* at a given y, so
// the height math upstream stays unit-testable without dragging block.h
// around. the driver calls fill_block per y, or fill_range for a whole span.

// soil depth: how many blocks of dirt sit under grass before stone takes over.
// flat ground gets a full layer, steep faces get thinner soil so cliffs show
// rock. clamped to at least 1 unless underwater rock.
int heightmap_fill_soil_depth(const heightmap_params *p,
                              const heightmap_column *col);

// the block at a single world y in this column. returns a block_id-compatible
// int. air above the surface (and above the waterline), water in between.
int heightmap_fill_block(const heightmap_params *p,
                         const heightmap_column *col, int y);

// fill a contiguous [y0,y1] span into out[], indexed out[y-y0]. returns the
// number of non-air blocks written, handy for the mesher's occupancy guess.
int heightmap_fill_range(const heightmap_params *p, const heightmap_column *col,
                         int y0, int y1, uint8_t *out);

#endif
