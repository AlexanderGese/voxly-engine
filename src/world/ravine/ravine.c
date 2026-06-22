#include "ravine.h"

#include "ravine_spawn.h"
#include "ravine_path.h"
#include "ravine_strata.h"
#include "ravine_resolve.h"
#include "ravine_column.h"
#include "ravine_detail.h"
#include "../../util/log.h"
#include <stdlib.h>

ravine_ctx *ravine_create(uint32_t world_seed, int sea_level) {
    ravine_ctx *ctx = calloc(1, sizeof *ctx);
    if (!ctx) return NULL;
    ctx->params = ravine_default_params(world_seed, sea_level);
    ctx->field = ravine_field_create();
    if (!ctx->field) {
        LOGE("ravine: ctx field alloc failed");
        free(ctx);
        return NULL;
    }
    return ctx;
}

void ravine_destroy(ravine_ctx *ctx) {
    if (!ctx) return;
    ravine_field_destroy(ctx->field);
    free(ctx);
}

void ravine_set_params(ravine_ctx *ctx, const ravine_params *p) {
    if (!ctx || !p) return;
    ctx->params = *p;
}

// merge two mask-stat structs (one per ravine) into the running total.
static void accum_mask(ravine_mask_stats *acc, const ravine_mask_stats *add) {
    acc->cells_cut   += add->cells_cut;
    acc->wall_cells  += add->wall_cells;
    acc->floor_cells += add->floor_cells;
    acc->rim_cells   += add->rim_cells;
    if (add->deepest > acc->deepest) acc->deepest = add->deepest;
}

ravine_report ravine_carve_chunk(ravine_ctx *ctx, chunk *c) {
    ravine_report rep;
    rep.ravines = 0;
    rep.mask   = (ravine_mask_stats){0, 0, 0, 0, 0};
    rep.carve  = (ravine_carve_stats){0, 0, 0, 0};
    rep.detail = (ravine_detail_stats){0, 0, 0};

    const ravine_params *p = &ctx->params;

    // 1. which ravines could reach this chunk?
    ravine_anchor anchors[RAVINE_MAX_ANCHORS];
    int na = ravine_spawn_collect(p, c->cx, c->cz, anchors, RAVINE_MAX_ANCHORS);
    if (na == 0) return rep;   // common case: nothing nearby, bail cheap.

    // set up the field for this region.
    ravine_origin origin;
    origin.base_x   = c->cx * CHUNK_SIZE_X;
    origin.base_z   = c->cz * CHUNK_SIZE_Z;
    origin.chunk_cx = c->cx;
    origin.chunk_cz = c->cz;
    ravine_field_reset(ctx->field, origin);

    // 3. surface scan first — the resolve reads heights out of it.
    ravine_column_scan(ctx->field, c);

    // strata is per-ravine. when ravines overlap (rare) the carve re-skins with
    // whichever table we built last; tracking the dominant ravine per cell isnt
    // worth the bookkeeping for how seldom they cross.
    ravine_strata strata;
    int strata_ready = 0;
    int top_y = WORLD_SEA_LEVEL + 32;   // strata table covers up past the surface

    for (int i = 0; i < na; i++) {
        ravine_path path;

        // anchor the floor near the local surface under the ravine head.
        int hx, hz;
        int afx = (int)anchors[i].anchor_x;
        int afz = (int)anchors[i].anchor_z;
        int floor_top = p->sea_level;
        if (ravine_field_from_world(ctx->field, afx, afz, &hx, &hz))
            floor_top = ravine_field_surface(ctx->field, hx, hz);

        ravine_path_build(&path, p, anchors[i].anchor_x, anchors[i].anchor_z,
                          floor_top, anchors[i].stream);

        // cheap reject before the per-cell resolve loop spins up.
        int rwx0 = origin.base_x - RAVINE_PAD;
        int rwz0 = origin.base_z - RAVINE_PAD;
        int rwx1 = origin.base_x + CHUNK_SIZE_X + RAVINE_PAD;
        int rwz1 = origin.base_z + CHUNK_SIZE_Z + RAVINE_PAD;
        if (!ravine_path_touches(&path, (float)rwx0, (float)rwz0,
                                 (float)rwx1, (float)rwz1))
            continue;

        ravine_mask_stats ms = ravine_resolve(ctx->field, &path, p);
        accum_mask(&rep.mask, &ms);

        if (ms.cells_cut > 0) {
            // roll this ravine's strata so the carve has bands to face with.
            // covers from the bedrock guard up past the surface.
            ravine_strata_build(&strata, p, p->min_floor_y, top_y,
                                anchors[i].stream);
            strata_ready = 1;
            rep.ravines++;
        }
    }

    if (!strata_ready || !ctx->field->dirty) return rep;

    // 5. stamp it into the chunk, then 6. rough the walls up.
    rep.carve = ravine_carve_apply(c, ctx->field, &strata, p);
    rep.detail = ravine_detail_apply(c, ctx->field, &strata, p);

    if (rep.carve.air_set > 0) {
        c->dirty = 1;   // needs a remesh now that we ate a hole in it.
        LOGD("ravine: cx=%d cz=%d carved %d air, %d water, %d strata, "
             "%d juts, %d shaves, depth %d",
             c->cx, c->cz, rep.carve.air_set, rep.carve.water_set,
             rep.carve.strata_faced, rep.detail.juts, rep.detail.shaves,
             rep.mask.deepest);
    }
    return rep;
}
