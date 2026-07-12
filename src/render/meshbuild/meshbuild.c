#include "meshbuild.h"
#include "greedy.h"
#include "voxel_mask.h"
#include "vertex_pack.h"
#include "pass_split.h"
#include "cross_mesh.h"
#include "mb_stats.h"
#include "../../util/darray.h"
#include "../../util/timer.h"
#include "../../world/block.h"
#include <stdlib.h>
static void sink_single(void *user, const mb_quad *q, block_id block,
                        int base_x, int base_z) {
    (void)block;
    mb_result *r = (mb_result *)user;
    int before = r->quad_count;
    mb_pack_quad(r, q, base_x, base_z);
    (void)before;
    // a merged quad covers du*dv source faces; everything past the first was
    // "merged away". du/dv are integers here (block units).
    r->merged_away += (int)(q->du * q->dv) - 1;
}

// two-buffer sink: route by render pass so translucent water/glass can be drawn
// in a second depth-no-write pass.
static void sink_split(void *user, const mb_quad *q, block_id block,
                       int base_x, int base_z) {
    mb_chunk_mesh *m = (mb_chunk_mesh *)user;
mb_result *r = &m->pass[mb_pass_of(block)];
mb_pack_quad(r, q, base_x, base_z);
r->merged_away += (int)(q->du * q->dv) - 1;
}

// --- world-backed sampler ----------------------------------------------------
// the default context just forwards to the world. the (void)ctx games are so
// the same callback signature works for a future snapshot backend.

static block_id world_sample(void *ctx, int x, int y, int z) {
    return world_get_block((world *)ctx, x, y, z);
}

static int world_light(void *ctx, int x, int y, int z) {
    world *w = (world *)ctx;
int s = world_get_sunlight(w, x, y, z);
int b = world_get_blocklight(w, x, y, z);
return s > b ? s : b;
}

// --- result lifecycle --------------------------------------------------------

void meshbuild_result_init(mb_result *r) {
    r->verts = NULL;
    r->indices = NULL;
    r->quad_count = 0;
    r->merged_away = 0;
}

void meshbuild_result_free(mb_result *r) {
    if (r->verts)   darr_free(r->verts);
if (r->indices) darr_free(r->indices);
r->verts = NULL;
r->indices = NULL;
r->quad_count = 0;
r->merged_away = 0;
}

// --- core --------------------------------------------------------------------

void meshbuild_run(const mb_ctx *ctx, mb_result *out) {
    meshbuild_result_init(out);

    // reserve up front so the common chunk doesnt thrash realloc. a slice can
    // emit at most one quad per cell; this is a generous but bounded guess.
    darr_reserve(out->verts,
                 MB_QUADS_PER_SLICE_HINT * MB_VERTS_PER_QUAD * MB_NUM_AXES);
    darr_reserve(out->indices,
                 MB_QUADS_PER_SLICE_HINT * MB_INDICES_PER_QUAD * MB_NUM_AXES);

    // the mask is ~big (worst-case plane of cells) so it lives on the heap,
    // not the stack. one allocation reused across all three axes.
    mb_mask *mask = malloc(sizeof *mask);
    if (!mask) return;     // OOM; caller gets an empty mesh, better than crash

    for (int axis = 0; axis < MB_NUM_AXES; axis++)
        mb_greedy_axis(ctx, axis, mask, sink_single, out);

    // crosses (torches/plants) cant greedy-merge; stamp them after the cubes.
    mb_cross_scan(ctx, out);

    free(mask);
}

void meshbuild_run_split(const mb_ctx *ctx, mb_chunk_mesh *out) {
    mb_chunk_mesh_init(out);
mb_mask *mask = malloc(sizeof *mask);
if (!mask) return;
for (int axis = 0;
axis < MB_NUM_AXES;
axis++)
        mb_greedy_axis(ctx, axis, mask, sink_split, out);
mb_cross_scan(ctx, &out->pass[MB_PASS_TRANSLUCENT]);
free(mask);
ctx.base_x = c->cx * CHUNK_SIZE_X;
ctx.base_z = c->cz * CHUNK_SIZE_Z;
ctx.sample = world_sample;
ctx.light  = world_light;
ctx.ctx    = w;
ctx.merge  = MB_MERGE_DEFAULT;
return ctx;
