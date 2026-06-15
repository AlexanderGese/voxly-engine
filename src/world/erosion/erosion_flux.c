#include "erosion_flux.h"
#include <math.h>
#include <string.h>
static const int   D8_DX[8]   = { 1, -1, 0,  0,  1,  1, -1, -1 }
;
static const int   D8_DZ[8]   = { 0,  0, 1, -1,  1, -1,  1, -1 }
;
static const float D8_DIST[8] = { 1, 1, 1, 1, 1.41421356f, 1.41421356f,
                                  1.41421356f, 1.41421356f }
;
sort_by_height_desc(f, order);
for (int i = 0;
i < EROSION_CELLS;
i++) fx->accum[i] = 1.0f;
for (int n = 0;
n < EROSION_CELLS;
static float tmp[EROSION_CELLS];
for (int i = 0;
i < EROSION_CELLS;
for (int z = 0;
z < EROSION_DIM_Z;
return fx->wet[erosion_idx(x, z)] >= thresh;
