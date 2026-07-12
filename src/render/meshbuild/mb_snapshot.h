#ifndef RENDER_MESHBUILD_MB_SNAPSHOT_H
#define RENDER_MESHBUILD_MB_SNAPSHOT_H

// a flattened, padded copy of a chunk and the one-block skirt around it that
// the mesher actually reads. the point is threaded meshing: copy everything the
// build needs under the world lock once (fast, just memcpy-ish), then release
// the lock and let a worker grind on the snapshot without anyone mutating the
// world out from under it.
//
// the skirt is 1 block on each side so neighbour reads for face culling, ao and
// light never go out of bounds. that makes the snapshot
// (SIZE_X+2) x SIZE_Y(+2) x (SIZE_Z+2) cells. y also gets padded so faces at
// y=0 / y=top sample a sane (air) neighbour.

#include "mb_types.h"
#include "../../world/world.h"

#define MB_SNAP_W (CHUNK_SIZE_X + 2)
#define MB_SNAP_H (CHUNK_SIZE_Y + 2)
#define MB_SNAP_D (CHUNK_SIZE_Z + 2)
#define MB_SNAP_VOL (MB_SNAP_W * MB_SNAP_H * MB_SNAP_D)

typedef struct {
    int      base_x, base_z;     // world coords of the chunk's (0,0,0)
    block_id block[MB_SNAP_VOL];
    uint8_t  light[MB_SNAP_VOL]; // combined max(sun,block) light 0..15
} mb_snapshot;

// fill a snapshot from the world. reads the target chunk plus its skirt. this
// is the only function that needs the world; everything downstream runs on the
// snapshot. allocate the snapshot however you like — its ~700KB so probably
// heap, not stack.
void mb_snapshot_fill(mb_snapshot *s, world *w, chunk *c);

// build an mb_ctx whose callbacks read the snapshot. valid as long as `s` is.
mb_ctx mb_snapshot_ctx(const mb_snapshot *s, int merge);

#endif
