#ifndef RENDER_MESHBUILD_MB_CONFIG_H
#define RENDER_MESHBUILD_MB_CONFIG_H

// internal tunables for the chunk mesh builder. these are knobs that only
// the meshbuild module cares about, so they dont live in the global config.h.
// if you find yourself wanting to expose one of these to the game, move it.

#include "../../config.h"

// how many quads we expect a "busy" chunk slice to produce. used to pick the
// initial scratch capacity so the common case never reallocs. measured this
// once on a hilly seed and 96 was the 95th percentile, rounded up.
#define MB_QUADS_PER_SLICE_HINT   128

// each greedy quad expands to 4 verts / 6 indices. we emit indexed geometry
// from the builder and let the caller decide whether to keep the index buffer.
#define MB_VERTS_PER_QUAD          4
#define MB_INDICES_PER_QUAD        6

// the three sweep axes. greedy meshing runs one pass per axis, two directions
// each, so six "face directions" total — same order the rest of render uses:
// 0 = +x, 1 = -x, 2 = +y, 3 = -y, 4 = +z, 5 = -z
#define MB_AXIS_X   0
#define MB_AXIS_Y   1
#define MB_AXIS_Z   2
#define MB_NUM_AXES 3
#define MB_NUM_FACES 6

// ao is quantised to 4 levels (0..3) like everyone elses. 3 = fully lit.
// we keep it as a small int through the pipeline and only convert to a float
// multiplier at pack time.
#define MB_AO_LEVELS 4
#define MB_AO_MAX    3

// light gets the same MAX_LIGHT range as the world. we clamp the darkest
// visible face to this so caves arent pure black voids you cant navigate.
#define MB_MIN_FACE_LIGHT 3

// greedy merging can be disabled per-build (handy for debugging t-junctions).
// when off the builder still runs the same path, it just never merges spans.
#define MB_MERGE_DEFAULT 1

// a slice is one plane of the chunk perpendicular to the sweep axis. the
// largest plane is x/z = 16*16, y is 16*128 etc. we size the mask to the max.
#define MB_MASK_MAX_DIM \
    (CHUNK_SIZE_Y > CHUNK_SIZE_X ? CHUNK_SIZE_Y : CHUNK_SIZE_X)

#endif
