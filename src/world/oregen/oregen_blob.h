#ifndef WORLD_OREGEN_BLOB_H
#define WORLD_OREGEN_BLOB_H

#include "oregen_types.h"
#include "oregen_buffer.h"

// blob/vein geometry. takes a seeded oregen_vein and expands it into cells
// in the buffer. three shapes share this file because they're variations on
// the same fuzzy-ellipsoid idea:
//
// BLOB    - a cluster of overlapping fuzzy spheres along a short jittered
// path. the bread-and-butter ore lump.
// VEIN    - a long thin wandering worm, thickness ~1-2. stringy deposits.
// POCKET  - a single near-spherical lump, tight, for rare ores.
//
// every builder respects vein->size as a soft voxel budget and squishes
// vertically by ore squish so veins lie flatter than they are wide.

// expand one vein into the buffer. returns the number of cells emitted.
int oregen_blob_build(oregen_buf *buf, const oregen_vein *v);

// the individual shape builders, exposed mostly for testing. the dispatch
// in oregen_blob_build just routes by v->shape.
int oregen_blob_sphere_cluster(oregen_buf *buf, const oregen_vein *v);
int oregen_blob_worm(oregen_buf *buf, const oregen_vein *v);
int oregen_blob_pocket(oregen_buf *buf, const oregen_vein *v);

#endif
