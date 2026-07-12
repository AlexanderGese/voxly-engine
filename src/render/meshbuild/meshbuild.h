#ifndef RENDER_MESHBUILD_MESHBUILD_H
#define RENDER_MESHBUILD_MESHBUILD_H

// public entry point for the chunk mesh builder.
//
// the old render/mesher.c walks every block and emits a quad per visible face
// straight into GL. this module does the same job but greedy-meshed, with
// proper per-vertex ambient occlusion and smoothed lighting, and it produces a
// plain cpu buffer (indexed) instead of touching GL — so it can run on a worker
// thread and the main thread just uploads the result.
//
// usage:
// mb_result res;
// meshbuild_chunk(world, c, &res);
// ... upload res.verts / res.indices ...
// meshbuild_result_free(&res);

#include "mb_types.h"
#include "pass_split.h"
#include "../../world/world.h"

// build a full chunk mesh from the world. neighbour reads cross chunk borders
// via world_get_block, so make sure the 4 neighbour chunks are generated first
// or border faces will be wrong (this is the same caveat the old mesher had).
void meshbuild_chunk(world *w, chunk *c, mb_result *out);

// pass-split variant: opaque and translucent geometry end up in separate
// buffers so the renderer can draw water/glass in a second pass. this is the
// one the live renderer should prefer.
void meshbuild_chunk_split(world *w, chunk *c, mb_chunk_mesh *out);

// lower-level variant: build from an explicit context. lets callers feed a
// flattened/snapshotted world for threaded meshing without holding the world
// lock the whole time.
void meshbuild_run(const mb_ctx *ctx, mb_result *out);

// pass-split build from an explicit context (for threaded meshing off a
// snapshot). caller inits nothing; out is fully initialised here.
void meshbuild_run_split(const mb_ctx *ctx, mb_chunk_mesh *out);

// initialise an empty result (zeroed darrays). meshbuild_chunk/run call this
// for you; exposed for callers that want to inspect a result before building.
void meshbuild_result_init(mb_result *r);

// free the darrays a result owns. safe to call twice.
void meshbuild_result_free(mb_result *r);

#endif
