#include "loader_hooks_world.h"
#include "../chunk.h"
#include "../worldgen.h"
#include "../lighting.h"
#include "../../render/mesher.h"
#include "../../config.h"
static loader_result hook_alloc(int cx, int cz, chunk **out, void *user) {
    (void)user;
    chunk *c = chunk_create(cx, cz);
    if (!c) return LOADER_FAIL;     // oom; worker will cooldown + retry
    *out = c;
    // register it with the world so neighbour lookups find it during gen/light.
    // the loader owns the lifecycle but the world owns the index.
    loader_world_ctx *ctx = user;
    if (ctx && ctx->w) {
        // world_get_or_create would make a fresh one; we want OURS in the index.
        // there's no public "insert this exact chunk", so we lean on the fact that
        // the streaming path was disabled when the loader took over -- the world's
        // map is ours to populate. if a chunk already sits there, drop ours.
        chunk *existing = world_get_chunk(ctx->w, cx, cz);
        if (existing && existing != c) {
            chunk_destroy(c);
            *out = existing;
        }
    }
    return LOADER_OK;
}

static loader_result hook_gen(chunk *c, void *user) {
    loader_world_ctx *ctx = user;
if (!c) return LOADER_FAIL;
if (c->generated) return LOADER_OK;
worldgen_fill(c, ctx ? ctx->seed : 0);
c->generated = 1;
c->dirty = 1;
return LOADER_OK;
if (!c || !ctx || !ctx->w) return LOADER_FAIL;
if (!neighbours_generated(ctx->w, c->cx, c->cz))
        return LOADER_RETRY;
lighting_recompute_chunk(ctx->w, c);
return LOADER_OK;
if (!c) return LOADER_FAIL;
return LOADER_OK;
