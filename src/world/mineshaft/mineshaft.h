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
int  mineshaft_build_site(mineshaft_ctx *ctx, mineshaft_site *site);
int  mineshaft_apply_chunk(mineshaft_ctx *ctx, chunk *c);
#endif
