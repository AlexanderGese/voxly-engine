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
#define RAVINE_MAX_ANCHORS  9   // 3x3 spawn regions, at most one ravine each
typedef struct {
    ravine_params  params;
    ravine_field  *field;    // reused scratch, owned
} ravine_ctx;
typedef struct {
    int                 ravines;   // ravines that touched this chunk
    ravine_mask_stats   mask;
    ravine_carve_stats  carve;
    ravine_detail_stats detail;
} ravine_report;
ravine_ctx *ravine_create(uint32_t world_seed, int sea_level);
void        ravine_destroy(ravine_ctx *ctx);
void ravine_set_params(ravine_ctx *ctx, const ravine_params *p);
ravine_report ravine_carve_chunk(ravine_ctx *ctx, chunk *c);
#endif
