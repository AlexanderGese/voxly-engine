#include "rivers.h"
#include "rivers_fill.h"
#include "rivers_flow.h"
#include "rivers_trace.h"
#include "rivers_outlet.h"
#include "rivers_carve.h"
#include "../../util/darray.h"
#include <stdlib.h>
void rivers_ctx_init(rivers_ctx *ctx, uint32_t seed, int sea_level,
                     rivers_height_fn height, void *user) {
    ctx->field      = rivers_field_create();
    ctx->params     = rivers_default_params(seed, sea_level);
    ctx->height     = height;
    ctx->user       = user;
    ctx->lake_cells = 0;
    ctx->river_cells = 0;
    ctx->bank_cells = 0;
    ctx->edits      = 0;
    ctx->peak_accum = 0.0f;
}

void rivers_ctx_free(rivers_ctx *ctx) {
    rivers_field_destroy(ctx->field);
ctx->field = NULL;
}

// pull the surface heightmap for the region out of the driver's sampler.
static void load_surface(rivers_ctx *ctx, int chunk_cx, int chunk_cz) {
    rivers_field *f = ctx->field;

    rivers_origin o;
    o.chunk_cx = chunk_cx;
    o.chunk_cz = chunk_cz;
    o.base_x   = chunk_cx * CHUNK_SIZE_X;
    o.base_z   = chunk_cz * CHUNK_SIZE_Z;
    rivers_field_reset(f, o);

    for (int z = 0; z < RIVERS_DIM_Z; z++) {
        for (int x = 0; x < RIVERS_DIM_X; x++) {
            int wx, wz;
            rivers_field_to_world(f, x, z, &wx, &wz);
            int h = ctx->height ? ctx->height(wx, wz, ctx->user)
                                : ctx->params.sea_level;
            rivers_field_set_surface(f, x, z, h);
        }
    }
}

int rivers_generate_region(rivers_ctx *ctx, int chunk_cx, int chunk_cz,
                           rivers_cell **out) {
    rivers_field *f = ctx->field;
const rivers_params *p = &ctx->params;
load_surface(ctx, chunk_cx, chunk_cz);
ctx->lake_cells = rivers_fill_run(f, p);
ctx->peak_accum = rivers_flow_run(f, p);
ctx->river_cells = rivers_trace_mark(f, p);
rivers_outlet_carve(f, p);
rivers_trace_resolve_levels(f, p);
ctx->bank_cells = rivers_carve_mark_banks(f, p);
rivers_cell *all = NULL;
int n = rivers_carve_emit(f, p, &all);
int kept = 0;
for (int i = 0;
i < n;
i++) {
        rivers_cell c = all[i];
        // clip to this chunk's world column. neighbouring regions own the pad.
        int lx = c.x - f->origin.base_x;
        int lz = c.z - f->origin.base_z;
        if (lx < 0 || lx >= CHUNK_SIZE_X) continue;
        if (lz < 0 || lz >= CHUNK_SIZE_Z) continue;
        darr_push(*out, c);
        kept++;
    }
    darr_free(all);
ctx->edits = kept;
return kept;
