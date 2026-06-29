#ifndef WORLD_TREEGEN_BUFFER_H
#define WORLD_TREEGEN_BUFFER_H

#include "treegen_types.h"
#include "treegen_rand.h"

// owned, growable voxel buffer for plant output. mirrors structgen_buffer on
// purpose so the deco driver can drain either with the same loop. it also tracks
// a local bounding box as you add, which the deco pass uses to skip plants whose
// footprint can't reach the chunk it's filling.

typedef struct treegen_buffer {
    treegen_voxel *items;
    int count;
    int cap;

    // running local-space bounds, inclusive. valid once count > 0.
    int min_x, min_y, min_z;
    int max_x, max_y, max_z;
} treegen_buffer;

void treegen_buffer_init(treegen_buffer *b);
void treegen_buffer_free(treegen_buffer *b);
void treegen_buffer_reset(treegen_buffer *b);   // keep storage

// add one voxel in plant-local space. grows as needed. returns 1, 0 on oom.
// air ids are dropped silently so callers can be lazy about clearing.
int  treegen_buffer_add(treegen_buffer *b, int x, int y, int z, block_id id);

// stamp a filled axis-aligned sphere of leaves centered at (cx,cy,cz). cells
// roll against density (0..100) so the clump looks ragged instead of a beachball.
// returns count added. rng may be NULL for a solid sphere.
int  treegen_buffer_blob(treegen_buffer *b, int cx, int cy, int cz, int radius,
                         block_id id, int density, treegen_rng *rng);

// vertical wood column from y0..y1 inclusive with the given radius (chebyshev),
// for thick trunks. returns count added.
int  treegen_buffer_column(treegen_buffer *b, int cx, int y0, int y1, int cz,
                           int radius, block_id id);

#endif
