#ifndef WORLD_STRUCTGEN_H
#define WORLD_STRUCTGEN_H

#include "structgen_types.h"
#include "structgen_buffer.h"
#include "../chunk.h"

// public face of the structure generator. the worldgen driver calls this once
// per chunk after terrain+surface are filled. it figures out if the chunk hosts
// (or is touched by) a structure, builds the voxels, and stamps the ones that
// land inside this chunk. structures can straddle chunk borders, so we scan a
// small neighborhood of candidate origins, not just this chunk.

// height query the host engine provides: surface y at a world column. lets the
// generator sit structures on the ground without owning the heightmap.
typedef int (*structgen_height_fn)(int wx, int wz, void *user);

typedef struct {
    structgen_config    cfg;
    uint32_t            world_seed;
    structgen_height_fn height;     // may be NULL -> falls back to sea level
    void               *height_user;
    structgen_buffer    scratch;    // reused across calls to avoid churn
} structgen_ctx;

void structgen_ctx_init(structgen_ctx *ctx, uint32_t world_seed,
                        structgen_height_fn height, void *height_user);
void structgen_ctx_free(structgen_ctx *ctx);

// build a single resolved site into ctx->scratch. returns voxels emitted.
int  structgen_build_site(structgen_ctx *ctx, const structgen_site *site);

// the main entry point. populate `chunk` with any structure voxels that fall
// inside it, considering structures rooted in nearby chunks too. returns how
// many blocks were written into this chunk.
int  structgen_apply_chunk(structgen_ctx *ctx, chunk *c);

#endif
