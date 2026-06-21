#include "mineshaft.h"
#include "mineshaft_place.h"
#include "mineshaft_grid.h"
#include "mineshaft_maze.h"
#include "mineshaft_build.h"
#include "mineshaft_box.h"
#include "../../config.h"

// how many regions out from the current chunk we scan for shaft anchors. a full
// grid is ~gdim*corridor_len ~= 60 blocks ~= 4 chunks wide and a region is ~12
// chunks, so a shaft can only reach in from the adjacent region. a 1-ring scan
// covers it; most candidates miss the placement roll and cost nothing.
#define SCAN_REGIONS   1

void mineshaft_ctx_init(mineshaft_ctx *ctx, uint32_t world_seed,
                        mineshaft_height_fn height, void *height_user) {
    ctx->cfg         = mineshaft_config_default();
    ctx->world_seed  = world_seed;
    ctx->height      = height;
    ctx->height_user = height_user;
    mineshaft_buffer_init(&ctx->scratch);
}

void mineshaft_ctx_free(mineshaft_ctx *ctx) {
    mineshaft_buffer_free(&ctx->scratch);
}

static int ground_at(mineshaft_ctx *ctx, int wx, int wz) {
    if (ctx->height) return ctx->height(wx, wz, ctx->height_user);
    return WORLD_SEA_LEVEL;   // flatworld fallback
}

int mineshaft_build_site(mineshaft_ctx *ctx, mineshaft_site *site) {
    // resolve the corridor floor from the surface and configured depth.
    int ground = ground_at(ctx, site->anchor_x, site->anchor_z);
    site->floor_y = ground - ctx->cfg.depth_below;
    if (site->floor_y < ctx->cfg.depth_min) site->floor_y = ctx->cfg.depth_min;

    mineshaft_buffer_reset(&ctx->scratch);

    // 1. abstract maze.
    int gdim = mineshaft_build_grid_dim(&ctx->cfg);
    mineshaft_grid grid;
    mineshaft_grid_init(&grid, gdim, gdim);

    mineshaft_rng rng;
    mineshaft_rng_seed(&rng, site->seed);
    mineshaft_maze_carve(&grid, &ctx->cfg, &rng);
    mineshaft_maze_classify(&grid, &ctx->cfg, &rng);

    // 2. stamp the grid into voxels. the build rng is a fresh stream so tweaking
    // the maze rng doesn't reshuffle every support beam.
    mineshaft_rng build_rng;
    mineshaft_rng_seed(&build_rng, mineshaft_seed_mix(site->seed, 0xb1a5edu));
    return mineshaft_build_grid(&ctx->scratch, &grid, site, &ctx->cfg, &build_rng);
}

// world->local conversion + chunk clip. anything outside the chunk or the world
// y range is silently dropped.
static int stamp_into_chunk(const mineshaft_buffer *buf, chunk *c) {
    int base_x = c->cx * CHUNK_SIZE_X;
    int base_z = c->cz * CHUNK_SIZE_Z;
    int wrote = 0;

    for (int i = 0; i < buf->count; i++) {
        const mineshaft_voxel *v = &buf->items[i];
        int lx = v->x - base_x;
        int lz = v->z - base_z;
        if (lx < 0 || lx >= CHUNK_SIZE_X) continue;
        if (lz < 0 || lz >= CHUNK_SIZE_Z) continue;
        if (v->y < 0 || v->y >= CHUNK_SIZE_Y) continue;
        chunk_set_block(c, lx, v->y, lz, v->id);
        wrote++;
    }
    return wrote;
}

// quick reject: does this site's xz footprint overlap the chunk at all? saves
// building+stamping a whole shaft we'd then clip entirely away.
static int site_touches_chunk(mineshaft_ctx *ctx, const mineshaft_site *site,
                              const chunk *c) {
    int gdim  = mineshaft_build_grid_dim(&ctx->cfg);
    int pitch = ctx->cfg.corridor_len;
    int half  = (gdim * pitch) / 2 + pitch;   // +pitch slack for room overhang
    mineshaft_box foot = mineshaft_box_make(
        site->anchor_x - half, 0, site->anchor_z - half,
        site->anchor_x + half, 1, site->anchor_z + half);

    int bx = c->cx * CHUNK_SIZE_X, bz = c->cz * CHUNK_SIZE_Z;
    mineshaft_box cbox = mineshaft_box_make(bx, 0, bz,
                                            bx + CHUNK_SIZE_X, 1,
                                            bz + CHUNK_SIZE_Z);
    return mineshaft_box_overlaps(&foot, &cbox);
}

int mineshaft_apply_chunk(mineshaft_ctx *ctx, chunk *c) {
    int wrote = 0;
    int rc = ctx->cfg.region_size > 0 ? ctx->cfg.region_size : 12;

    // scan our region and the immediate ring so a shaft anchored next door can
    // still spill into us. dedupe by anchor: each region builds at most once.
    enum { MAXSEEN = (2 * SCAN_REGIONS + 1) * (2 * SCAN_REGIONS + 1) };
    int seen_x[MAXSEEN], seen_z[MAXSEEN];
    int nseen = 0;

    for (int dz = -SCAN_REGIONS; dz <= SCAN_REGIONS; dz++) {
        for (int dx = -SCAN_REGIONS; dx <= SCAN_REGIONS; dx++) {
            int ccx = c->cx + dx * rc;
            int ccz = c->cz + dz * rc;

            mineshaft_site site;
            if (!mineshaft_pick(&ctx->cfg, ccx, ccz, ctx->world_seed, &site))
                continue;

            // dedupe: the same region resolves identically from any chunk in it.
            int dup = 0;
            for (int s = 0; s < nseen; s++)
                if (seen_x[s] == site.anchor_x && seen_z[s] == site.anchor_z) {
                    dup = 1; break;
                }
            if (dup) continue;
            seen_x[nseen] = site.anchor_x;
            seen_z[nseen] = site.anchor_z;
            nseen++;

            if (!site_touches_chunk(ctx, &site, c)) continue;

            mineshaft_build_site(ctx, &site);
            wrote += stamp_into_chunk(&ctx->scratch, c);
        }
    }

    if (wrote) c->dirty = 1;   // changed blocks, mesher needs to re-run
    return wrote;
}
