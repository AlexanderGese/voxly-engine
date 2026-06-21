#ifndef WORLD_MINESHAFT_H
#define WORLD_MINESHAFT_H

#include <stdint.h>
#include "mineshaft_types.h"
#include "mineshaft_buffer.h"
#include "../chunk.h"

// public face of the abandoned-mineshaft generator. the worldgen driver calls
// mineshaft_apply_chunk once per chunk after terrain is filled. it scans a small
// neighbourhood of placement regions, builds any shaft that could reach into the
// chunk, and stamps the voxels that land inside. shafts straddle many chunks so
// the scan is region-granular. same shape as structgen so both can hang off the
// same driver loop.

// height query the host provides: surface y at a world column. lets us hang the
// shaft a fixed distance under the real ground without owning the heightmap.
typedef int (*mineshaft_height_fn)(int wx, int wz, void *user);

typedef struct {
    mineshaft_config    cfg;
    uint32_t            world_seed;
    mineshaft_height_fn height;       // may be NULL -> falls back to sea level
    void               *height_user;
    mineshaft_buffer    scratch;      // reused across calls to dodge churn
} mineshaft_ctx;

void mineshaft_ctx_init(mineshaft_ctx *ctx, uint32_t world_seed,
                        mineshaft_height_fn height, void *height_user);
void mineshaft_ctx_free(mineshaft_ctx *ctx);

// build a single resolved site into ctx->scratch. resolves the site's floor_y
// from the surface and config depth, then carves+stamps the maze into scratch.
// returns voxels emitted. exposed for tests and custom-stamping callers.
int  mineshaft_build_site(mineshaft_ctx *ctx, mineshaft_site *site);

// main entry point. populate `c` with any mineshaft voxels that fall inside it,
// considering shafts rooted in nearby regions too. returns blocks written.
int  mineshaft_apply_chunk(mineshaft_ctx *ctx, chunk *c);

#endif
