#include "loader_hooks_world.h"

#include "../chunk.h"
#include "../worldgen.h"
#include "../lighting.h"
#include "../../render/mesher.h"
#include "../../config.h"

// each hook is a thin shim: do the engine call, translate its (mostly void)
// outcome into a loader_result. the interesting logic is the RETRY decisions --
// light and mesh both need their neighbours present, and we'd rather wait a tick
// than build a seam we'll have to rebuild.

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
    if (c->generated) return LOADER_OK;   // already done (resumed slot)
    worldgen_fill(c, ctx ? ctx->seed : 0);
    c->generated = 1;
    c->dirty = 1;
    return LOADER_OK;
}

// a chunk can only be lit once its four edge-neighbours have terrain, otherwise
// sunlight leaks across the unfilled border. check, and RETRY if anyone's missing.
static int neighbours_generated(world *w, int cx, int cz) {
    static const int dx[4] = { 1, -1, 0, 0 };
    static const int dz[4] = { 0, 0, 1, -1 };
    for (int i = 0; i < 4; i++) {
        chunk *n = world_get_chunk(w, cx + dx[i], cz + dz[i]);
        if (!n || !n->generated) return 0;
    }
    return 1;
}

static loader_result hook_light(chunk *c, void *user) {
    loader_world_ctx *ctx = user;
    if (!c || !ctx || !ctx->w) return LOADER_FAIL;
    if (!neighbours_generated(ctx->w, c->cx, c->cz))
        return LOADER_RETRY;     // wait for the border to fill
    lighting_recompute_chunk(ctx->w, c);
    return LOADER_OK;
}

static loader_result hook_mesh(chunk *c, void *user) {
    loader_world_ctx *ctx = user;
    if (!c || !ctx || !ctx->w) return LOADER_FAIL;
    // the mesher culls faces against neighbours; if they arent meshed-ready (we
    // proxy that with "generated") we'd over-draw the seam. same gate as light.
    if (!neighbours_generated(ctx->w, c->cx, c->cz))
        return LOADER_RETRY;
    // refresh neighbour pointers so the mesher's face-cull sees current edges.
    world_update_neighbors(ctx->w);
    mesher_build_chunk(ctx->w, c);
    c->dirty = 0;
    return LOADER_OK;
}

// the mesher already uploads into the chunk vao/vbo in this engine, so the upload
// stage is a formality -- it exists so a future threaded mesher (cpu mesh on a
// worker, gl upload on main) slots in without touching the state machine.
static loader_result hook_upload(chunk *c, void *user) {
    (void)user;
    if (!c) return LOADER_FAIL;
    // nothing to do today; mesher_build_chunk already pushed to gl. when the
    // mesher splits cpu/gl this is where mesh_upload(&cpu_mesh) lands.
    return LOADER_OK;
}

loader_hooks loader_hooks_world(loader_world_ctx *ctx) {
    loader_hooks h;
    h.alloc  = hook_alloc;
    h.gen    = hook_gen;
    h.light  = hook_light;
    h.mesh   = hook_mesh;
    h.upload = hook_upload;
    h.user   = ctx;
    return h;
}
