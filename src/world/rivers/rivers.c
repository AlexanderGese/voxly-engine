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

    // 1. fill depressions and pond the basins into lakes.
    ctx->lake_cells = rivers_fill_run(f, p);

    // 2. flow directions + accumulation off the corrected surface.
    ctx->peak_accum = rivers_flow_run(f, p);

    // 3. threshold cells -> rivers, springs -> sources.
    ctx->river_cells = rivers_trace_mark(f, p);

    // 3b. notch lake outlets so the overflow becomes downstream river. do this
    // before resolving levels so the new channel gets a sane surface.
    rivers_outlet_carve(f, p);

    // 4. resolve a monotone water surface, no uphill water.
    rivers_trace_resolve_levels(f, p);

    // 5. flag the shores.
    ctx->bank_cells = rivers_carve_mark_banks(f, p);

    // 6. emit edits. we only stamp cells inside the real chunk footprint; the
    // pad existed purely so flow lined up across the seam.
    rivers_cell *all = NULL;
    int n = rivers_carve_emit(f, p, &all);

    int kept = 0;
    for (int i = 0; i < n; i++) {
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
}

rivers_wet rivers_query(const rivers_ctx *ctx, int wx, int wz, int *water_y) {
    int x, z;
    if (!rivers_field_from_world(ctx->field, wx, wz, &x, &z)) {
        if (water_y) *water_y = 0;
        return RIVERS_DRY;
    }
    int idx = rivers_field_idx(x, z);
    if (water_y) *water_y = ctx->field->water_y[idx];
    return (rivers_wet)ctx->field->wet[idx];
}
