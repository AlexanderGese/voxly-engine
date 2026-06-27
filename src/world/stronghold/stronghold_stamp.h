#ifndef WORLD_STRONGHOLD_STAMP_H
#define WORLD_STRONGHOLD_STAMP_H

#include "stronghold_buffer.h"
#include "../chunk.h"

// the bridge from the abstract voxel buffer to actual chunk storage. the
// generator produces a flat list of (world xyz, block) voxels; the worldgen
// driver calls this per chunk as chunks load, stamping only the voxels that
// fall inside that chunk. keeps the generator chunk-free while still letting
// the world place strongholds lazily, one chunk at a time.

// stamp every voxel in `buf` that lands inside chunk `c`. returns the number of
// blocks actually written. `overwrite_air` controls whether AIR voxels carve
// existing blocks (yes, for the hollowing) or are skipped.
int stronghold_stamp_chunk(const stronghold_buffer *buf, chunk *c, int overwrite_air);

// does the buffer touch chunk (cx,cz) at all? cheap reject so the driver can
// skip chunks the stronghold doesnt reach without scanning every voxel twice.
int stronghold_stamp_touches(const stronghold_buffer *buf, int cx, int cz);

// world-space bounding box of every voxel in the buffer. fills mins/maxs (each
// a 3-int array). returns 0 for an empty buffer (bounds left untouched).
int stronghold_stamp_bounds(const stronghold_buffer *buf, int *mins, int *maxs);

#endif
