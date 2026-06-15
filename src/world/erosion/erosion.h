#ifndef WORLD_EROSION_H
#define WORLD_EROSION_H

#include <stdint.h>
#include "erosion_types.h"
#include "erosion_field.h"
#include "../chunk.h"

// top-level erosion driver. the worldgen pipeline calls this after the base
// heightmap is laid down but before decoration (trees/ore), so rivers carve
// before anything sits in them. order of operations per tile:
//
// 1. load the float heightmap (+ a skirt of neighbour columns) into a field
// 2. seed per-cell rock hardness
// 3. hydraulic pass  -- rain droplets carve valleys, move sediment
// 4. thermal pass    -- talus slumps the fresh cliffs into scree
// 5. settle pass     -- loose sediment folds back into height
// 6. apply           -- reconcile the chunk's block columns to the new heights
//
// the whole thing is deterministic on (seed, tile origin) so regenerating a
// chunk gives the same river. that cost me a weekend of "why did the canyon
// move", hence the private rng streams everywhere.

// fill a field from a raw heightmap. `heights` is EROSION_CELLS floats in the
// field layout (skirt included); the caller samples worldgen for the skirt.
void erosion_load_field(erosion_field *f, const erosion_params *p,
                        int ox, int oz, const float *heights);

// run passes 2..5 on a loaded field. leaves the result in f->height/sediment.
// stats is optional. this is the pure number-crunching half, no blocks.
void erosion_run(erosion_field *f, const erosion_params *p, erosion_stats *st);

// convenience: snapshot the field's interior heights (the part that maps to a
// chunk) into an int array of pre-erosion tops for the apply stage. call it
// right after load, before erosion_run mutates anything.
void erosion_snapshot_tops(const erosion_field *f, int *old_height_out);

// full pipeline for one chunk: load -> run -> apply. `heights` is the field
// heightmap, `c` the already-generated chunk to reconcile. returns blocks
// changed. this is the one-call entry most of worldgen will use.
int erosion_process_chunk(chunk *c, const erosion_params *p,
                          int ox, int oz, const float *heights,
                          int sea_level, erosion_stats *st);

// zeroed stats, for callers that want to accumulate across tiles.
erosion_stats erosion_stats_zero(void);

#endif
