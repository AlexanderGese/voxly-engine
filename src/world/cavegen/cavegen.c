#include "cavegen.h"

#include "cavegen_grid.h"
#include "cavegen_column.h"
#include "cavegen_automata.h"
#include "cavegen_worm.h"
#include "cavegen_ravine.h"
#include "../../config.h"
#include "../../util/log.h"
#include <stdlib.h>

cavegen_ctx *cavegen_create(uint32_t world_seed) {
    cavegen_ctx *ctx = calloc(1, sizeof *ctx);
    if (!ctx) return NULL;
    ctx->params = cavegen_default_params(world_seed);
    ctx->grid = cavegen_grid_create();
    if (!ctx->grid) {
        free(ctx);
        return NULL;
    }
    return ctx;
}

void cavegen_destroy(cavegen_ctx *ctx) {
    if (!ctx) return;
    cavegen_grid_destroy(ctx->grid);
    free(ctx);
}

void cavegen_set_params(cavegen_ctx *ctx, const cavegen_params *p) {
    if (!ctx || !p) return;
    ctx->params = *p;
}

cavegen_report cavegen_carve_chunk(cavegen_ctx *ctx, chunk *c) {
    cavegen_report rep = {0};

    cavegen_origin origin;
    origin.chunk_cx = c->cx;
    origin.chunk_cz = c->cz;
    // world coord of the chunk's local (0,0) corner.
    origin.base_x = c->cx * CHUNK_SIZE_X;
    origin.base_z = c->cz * CHUNK_SIZE_Z;

    cavegen_grid_reset(ctx->grid, origin);

    // 1. heightmap so the crust check has something to bite on.
    cavegen_column_scan_chunk(ctx->grid, c);
    cavegen_column_fill_pad(ctx->grid);

    // 2. cellular automata: random fill then smooth into blobby rooms.
    cavegen_automata_seed(ctx->grid, &ctx->params);
    cavegen_automata_smooth(ctx->grid, &ctx->params);

    // 3. perlin worms thread tunnels through (and between) the rooms.
    cavegen_worm_spawn_all(ctx->grid, &ctx->params);

    // 3b. maybe a ravine, if this chunk drew the short straw.
    cavegen_ravine_maybe_spawn(ctx->grid, &ctx->params);

    // 4. connectivity: seal the little orphan bubbles.
    rep.connect = cavegen_connect_run(ctx->grid, &ctx->params);

    // 5. stamp into the chunk.
    rep.carve = cavegen_carve_apply(c, ctx->grid, &ctx->params);

    LOGD("cavegen chunk (%d,%d): %d regions, sealed %d, carved %d air / %d water",
         c->cx, c->cz, rep.connect.region_count, rep.connect.sealed_regions,
         rep.carve.air_set, rep.carve.water_set);

    return rep;
}
