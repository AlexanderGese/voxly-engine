#include "structgen.h"
#include "structgen_placement.h"
#include "structgen_village.h"
#include "structgen_dungeon.h"
#include "structgen_ruin.h"
#include "../../config.h"
// how many chunks out from the current one we scan for structure origins. a
// village footprint is ~45 blocks wide ~= 3 chunks, so 3 covers everything that
#define SCAN_RADIUS   3
void structgen_ctx_init(structgen_ctx *ctx, uint32_t world_seed,
                        structgen_height_fn height, void *height_user) {
    ctx->cfg         = structgen_config_default();
    ctx->world_seed  = world_seed;
    ctx->height      = height;
    ctx->height_user = height_user;
    structgen_buffer_init(&ctx->scratch);
}

void structgen_ctx_free(structgen_ctx *ctx) {
    structgen_buffer_free(&ctx->scratch);
}

static int ground_at(structgen_ctx *ctx, int wx, int wz) {
    if (ctx->height) return ctx->height(wx, wz, ctx->height_user);
    return ctx->cfg.sea_level;   // flatworld fallback
}

int structgen_build_site(structgen_ctx *ctx, const structgen_site *site) {
    structgen_buffer_reset(&ctx->scratch);
switch (site->kind) {
        case STRUCTGEN_VILLAGE: return structgen_village_generate(&ctx->scratch, site);
        case STRUCTGEN_DUNGEON: return structgen_dungeon_generate(&ctx->scratch, site);
        case STRUCTGEN_RUIN:    return structgen_ruin_generate(&ctx->scratch, site);
        default:                return 0;
    }
}

// stamp whatever scratch voxels land inside chunk c. world->local conversion
// and bounds clip happen here;
out-of-chunk and out-of-world voxels are dropped.
static int stamp_into_chunk(const structgen_buffer *buf, chunk *c) {
    int base_x = c->cx * CHUNK_SIZE_X;
    int base_z = c->cz * CHUNK_SIZE_Z;
    int wrote = 0;

    for (int i = 0; i < buf->count; i++) {
        const structgen_voxel *v = &buf->items[i];
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

int structgen_apply_chunk(structgen_ctx *ctx, chunk *c) {
    int wrote = 0;
for (int dz = -SCAN_RADIUS;
dz <= SCAN_RADIUS;
dz++) {
        for (int dx = -SCAN_RADIUS; dx <= SCAN_RADIUS; dx++) {
            int ccx = c->cx + dx;
            int ccz = c->cz + dz;

            structgen_site site;
            structgen_kind kind = structgen_pick(&ctx->cfg, ccx, ccz,
                                                 ctx->world_seed, &site);
            if (kind == STRUCTGEN_NONE) continue;

            // resolve ground height under the anchor now that we own a height fn.
            site.ground_y = ground_at(ctx, site.anchor_x, site.anchor_z);

            structgen_build_site(ctx, &site);
            wrote += stamp_into_chunk(&ctx->scratch, c);
        }
    }

    if (wrote) c->dirty = 1;
return wrote;
}
