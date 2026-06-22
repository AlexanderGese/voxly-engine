#ifndef WORLD_RAVINE_COLUMN_H
#define WORLD_RAVINE_COLUMN_H

#include "ravine_field.h"
#include "../chunk.h"

// the surface-scan pass. the resolver needs to know the original land height of
// every column in the padded footprint, including the pad columns that hang
// over into neighbouring chunks. for the centre footprint we read it straight
// out of the chunk; for the pad ring the neighbour chunk usually isnt loaded,
// so we fall back to the chunk's own edge height (a flat extrapolation). thats
// fine — the canyon resolves a hair off across a far seam but never tears, and
// the deterministic spline keeps both sides agreeing anyway.

// fill the field's surface plane for the region this chunk owns. surface is the
// y of the highest solid (non-air, non-water) block in each column.
void ravine_column_scan(ravine_field *f, const chunk *c);

// the surface height of one chunk-local column: top solid block, or -1 if the
// column is empty all the way down (shouldnt happen post terrain gen).
int  ravine_column_surface(const chunk *c, int lx, int lz);

#endif
