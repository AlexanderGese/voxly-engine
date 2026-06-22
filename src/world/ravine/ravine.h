#ifndef WORLD_RAVINE_H
#define WORLD_RAVINE_H
#include "ravine_types.h"
#include "ravine_field.h"
#include "ravine_carve.h"
#include "ravine_resolve.h"
#include "ravine_detail.h"
#include "../chunk.h"
// top-level ravine / canyon generator. worldgen calls ravine_carve_chunk()
// after surface terrain is laid down (it needs the heightmap) but before fine
// decoration. the pipeline per chunk:
//
// 1. collect nearby ravine anchors from the spawn-region grid   (spawn)
// 2. build each ravine's catmull-rom centreline + strata table   (path/strata)
// 3. scan the chunk for its surface heightmap into the field      (column)
// 4. resolve every centreline into the field: distance -> cut       (resolve)
// 5. stamp the field into the chunk as air/water/strata               (carve)
//
// steps 3-5 work on a padded scratch field so a ravine that clips a chunk
// border resolves identically from either side. one ravine_ctx owns the
// reusable field; create it once per worldgen worker, not per chunk.
#define RAVINE_MAX_ANCHORS  9   // 3x3 spawn regions, at most one ravine each
typedef struct {
    ravine_params  params;
    ravine_field  *field;    // reused scratch, owned
} ravine_ctx;
// rolled-up stats from one chunk's carve. mostly for logging/tests.
typedef struct {
    int                 ravines;   // ravines that touched this chunk
    ravine_mask_stats   mask;
    ravine_carve_stats  carve;
    ravine_detail_stats detail;
} ravine_report;
ravine_ctx *ravine_create(uint32_t world_seed, int sea_level);
void        ravine_destroy(ravine_ctx *ctx);
// override the defaults. copies the params in.
void ravine_set_params(ravine_ctx *ctx, const ravine_params *p);
// run the whole pipeline against one chunk, in place. safe on a chunk that
// already has terrain — only carves diggable solids, never bedrock. returns a
// report. a chunk with no nearby ravine comes back with ravines == 0 and is
// left untouched.
ravine_report ravine_carve_chunk(ravine_ctx *ctx, chunk *c);
#endif
