#ifndef WORLD_OREGEN_H
#define WORLD_OREGEN_H

#include "oregen_types.h"
#include "oregen_buffer.h"

// top-level ore generation driver. seed veins for a chunk, grow their
// blobs, and hand back the cells. the worldgen pass calls this after the
// stone column is filled but before lighting, then stamps the cells in
// wherever the target block is replaceable (usually stone only).
//
// this stays link-clean: it never includes chunk.h. the driver emits world
// coords into a buffer and the caller decides what to do with them. see
// oregen_stamp.h for the optional chunk integration glue.

// what an emitted cell is allowed to overwrite. veins should only eat into
// solid stone, not air pockets or other ores, or you get floating ore in
// caves. caller supplies the predicate; we never assume a world.
typedef int (*oregen_replace_fn)(block_id existing, void *ud);

// config knobs the driver reads. zero-init gives sane defaults via
// oregen_params_default.
typedef struct {
    uint32_t world_seed;
    int      max_veins;       // cap on seeded veins per chunk (safety)
    int      drop_underbudget;// 1 = skip veins whose roll fell below 1 cell
} oregen_params;

oregen_params oregen_params_default(uint32_t world_seed);

// generate ore cells for the chunk-sized column rooted at world
// (origin_x, origin_z). surface_y is the terrain top for clamping spawns
// (pass the max column height in the chunk, or CHUNK_SIZE_Y if unsure).
// fills buf. returns the number of cells emitted.
int oregen_generate_chunk(oregen_buf *buf, int origin_x, int origin_z,
                          int surface_y, const oregen_params *params);

// same but reports how many distinct veins actually contributed, via
// veins_out (may be NULL). useful for debug overlays.
int oregen_generate_chunk_ex(oregen_buf *buf, int origin_x, int origin_z,
                             int surface_y, const oregen_params *params,
                             int *veins_out);

#endif
