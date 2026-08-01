#ifndef ENTITY_SPAWN_MSPAWN_LOCAL_H
#define ENTITY_SPAWN_MSPAWN_LOCAL_H

#include <stdint.h>
#include "mspawn_types.h"
#include "../../world/chunk.h"

// per-chunk cache of standable columns. throwing darts and walking columns in
// the live world every cycle is fine at our mob counts, but once a chunk is
// loaded its surface barely changes, so we can precompute "here are the spots a
// mob could stand" once and sample from that. the driver still re-validates
// light at commit time (light moves with the sun), it just gets the candidate
// columns for free.
//
// a "spot" is the floor y of a solid-with-air-above column plus the cached
// daylight contribution. one chunk has at most CHUNK_SIZE_X*CHUNK_SIZE_Z
// surface spots, usually far fewer once you drop the buried/underwater ones.

#define MSPAWN_LOCAL_MAX (CHUNK_SIZE_X * CHUNK_SIZE_Z)

typedef struct {
    uint8_t lx, lz;     // local column 0..15
    uint8_t fy;         // floor y. world is 128 tall so this fits a byte fine
    uint8_t sun;        // cached sunlight at feet, 0..15
} mspawn_spot;

typedef struct {
    int         cx, cz;             // owning chunk address
    mspawn_spot spots[MSPAWN_LOCAL_MAX];
    int         count;
    int         valid;              // 0 until built
} mspawn_local;

// (re)build the spot list for a chunk. solid surfaces with MSPAWN_HEADROOM of
// air above, skipping water columns. cheap, single pass per column.
void mspawn_local_build(mspawn_local *lc, const chunk *c);

// resolve a spot's world position (voxel center, feet height). needs the chunk
// address the cache was built against.
vec3 mspawn_local_pos(const mspawn_local *lc, int i);

// pull a spot by index. returns 1 + fills floor y / sun, or 0 if out of range.
int  mspawn_local_spot(const mspawn_local *lc, int i, int *out_fy, int *out_sun);

#endif
