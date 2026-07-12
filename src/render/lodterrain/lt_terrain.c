#include "lt_terrain.h"

#include "lt_build.h"
#include "lt_mesh.h"
#include "lt_seam.h"
#include "lt_neighbor.h"
#include "../../math/aabb.h"
#include "../../config.h"
#include <stddef.h>

// ---- world-backed source -------------------------------------------------
// the builder reads the world only through these. nothing stops us swapping in
// a flattened snapshot later for a worker thread; today they just forward to
// the world api on the main thread.

static block_id src_sample(void *ctx, int wx, int wy, int wz) {
    world *w = ctx;
    if (wy < 0 || wy >= CHUNK_SIZE_Y) return BLOCK_AIR;
    return world_get_block(w, wx, wy, wz);
}

static int src_light(void *ctx, int wx, int wy, int wz) {
    world *w = ctx;
    if (wy < 0) return 0;
    if (wy >= CHUNK_SIZE_Y) return MAX_LIGHT;   // open sky above the world
    int bl = world_get_blocklight(w, wx, wy, wz);
    int sl = world_get_sunlight(w, wx, wy, wz);
    return bl > sl ? bl : sl;   // brighter of the two, matches the main mesher
}

static lt_source world_source(world *w, int cx, int cz) {
    lt_source s;
    s.base_x = cx * CHUNK_SIZE_X;
    s.base_z = cz * CHUNK_SIZE_Z;
    s.sample = src_sample;
    s.light  = src_light;
    s.ctx    = w;
    return s;
}

// ---- lifecycle -----------------------------------------------------------

void lt_terrain_init(lt_terrain *t, int build_budget) {
    lt_cache_init(&t->cache);
    lt_select_default(&t->select);
    t->build_budget = build_budget > 0 ? build_budget : 4;
    t->enabled = 1;
    lt_mesh_init(&t->scratch);

    t->stats.chunks_considered = 0;
    t->stats.builds_this_frame = 0;
    t->stats.builds_deferred   = 0;
    t->stats.drawn = 0;
    t->stats.tris_drawn = 0;
}

void lt_terrain_destroy(lt_terrain *t) {
    lt_mesh_free(&t->scratch);
    lt_cache_free(&t->cache);
}

void lt_terrain_invalidate(lt_terrain *t, int cx, int cz) {
    // ripple to the four neighbours too — a far edit can shift a seam, and a
    // stale stitch on the next chunk over is exactly the kind of crack this
    // whole subsystem exists to avoid.
    lt_neighbor_ripple(&t->cache, cx, cz);
}

// ---- update --------------------------------------------------------------

// the level a chunk *would* be selected at, used both for our own choice and to
// fill in a neighbour's level for the seam mask. mirrors the manager's logic so
// neighbours and self stay consistent — if they disagreed we'd skirt the wrong
// edges.
static int level_for(lt_terrain *t, int cx, int cz, vec3 cam) {
    float d = lt_select_chunk_dist(cx, cz, cam.x, cam.z);
    lt_cache_entry *e = lt_cache_find(&t->cache, cx, cz);
    int prev = e ? e->level : -1;
    return lt_select_pick_stable(&t->select, d, prev);
}

// gather the four horizontal neighbour levels for seam analysis. a missing
// chunk reports -1 so we don't skirt against the void.
static void neighbor_levels(lt_terrain *t, world *w, int cx, int cz,
                            vec3 cam, lt_neighbor_levels *out) {
    out->level[0] = world_get_chunk(w, cx + 1, cz) ? level_for(t, cx + 1, cz, cam) : -1; // px
    out->level[1] = world_get_chunk(w, cx - 1, cz) ? level_for(t, cx - 1, cz, cam) : -1; // nx
    out->level[2] = world_get_chunk(w, cx, cz + 1) ? level_for(t, cx, cz + 1, cam) : -1; // pz
    out->level[3] = world_get_chunk(w, cx, cz - 1) ? level_for(t, cx, cz - 1, cam) : -1; // nz
}

// per-chunk visit context. world_visit hands us one chunk at a time.
typedef struct {
    lt_terrain *t;
    world      *w;
    vec3        cam;
    float       dt;
} update_ctx;

static void visit_select(chunk *c, void *user) {
    update_ctx *uc = user;
    lt_terrain *t = uc->t;

    t->stats.chunks_considered++;

    int level = level_for(t, c->cx, c->cz, uc->cam);

    lt_neighbor_levels nb;
    neighbor_levels(t, uc->w, c->cx, c->cz, uc->cam, &nb);
    lt_seam_mask seam = lt_seam_compute(level, &nb);

    lt_cache_entry *e = lt_cache_get(&t->cache, c->cx, c->cz);
    if (!e) return;   // oom, skip — we'll get it next frame

    e->want_level = level;
    e->want_seam  = seam;

    // kick a morph whenever the target level moves, and advance any in flight.
    // the fade keeps the chunk's silhouette from snapping when it crosses a band.
    lt_morph_retarget(&e->morph, level);
    lt_morph_tick(&e->morph, uc->dt);

    // if the chunk's block data changed since we last meshed, the world marked
    // its dirty flag; fold that into our own dirty so we rebuild the lod too.
    if (c->dirty) e->dirty = 1;

    if (!lt_cache_needs_build(e)) return;

    // throttle: only so many rebuilds per frame. the rest keep their (slightly
    // stale) mesh until budget frees up. far chunks tolerate a frame of lag.
    if (t->stats.builds_this_frame >= t->build_budget) {
        t->stats.builds_deferred++;
        return;
    }

    lt_source src = world_source(uc->w, c->cx, c->cz);
    if (lt_build_chunk(&t->scratch, &src, level, &nb)) {
        lt_gpu_upload(&e->gpu, &t->scratch);
        lt_cache_commit(&t->cache, e, lt_mesh_tris(&t->scratch));
        t->stats.builds_this_frame++;
    }
}

void lt_terrain_update(lt_terrain *t, world *w, vec3 cam_pos, float dt) {
    if (!t->enabled) return;

    t->stats.chunks_considered = 0;
    t->stats.builds_this_frame = 0;
    t->stats.builds_deferred   = 0;

    update_ctx uc = { t, w, cam_pos, dt };
    world_visit(w, visit_select, &uc);

    // note: eviction of streamed-out chunks is driven by the renderer calling
    // lt_terrain_invalidate / relying on the world's own removal — we keep the
    // cache lean by dropping entries whose chunk vanished. cheap sweep below.
    hm_iter it;
    uint64_t key;
    void *val;
    hm_iter_init(&it, &t->cache.map);
    // collect-then-evict: can't mutate the map mid-iteration safely.
    // bounded by the cache size, which tracks the loaded set, so it's small.
    int to_evict_cx[64];
    int to_evict_cz[64];
    int n_evict = 0;
    while (hm_iter_next(&it, &key, &val) && n_evict < 64) {
        lt_cache_entry *e = val;
        if (!world_get_chunk(w, e->cx, e->cz)) {
            to_evict_cx[n_evict] = e->cx;
            to_evict_cz[n_evict] = e->cz;
            n_evict++;
        }
    }
    for (int i = 0; i < n_evict; i++)
        lt_cache_evict(&t->cache, to_evict_cx[i], to_evict_cz[i]);
}

// ---- render --------------------------------------------------------------

// world-space aabb for a chunk column, same convention as the cull module so a
// frustum reject here agrees with the one over there.
static aabb chunk_box(int cx, int cz) {
    vec3 mn = { (float)(cx * CHUNK_SIZE_X), 0.0f, (float)(cz * CHUNK_SIZE_Z) };
    vec3 mx = { mn.x + (float)CHUNK_SIZE_X, (float)CHUNK_SIZE_Y,
                mn.z + (float)CHUNK_SIZE_Z };
    return aabb_make(mn, mx);
}

void lt_terrain_render(lt_terrain *t, glid shader, mat4 vp) {
    if (!t->enabled) return;

    t->stats.drawn = 0;
    t->stats.tris_drawn = 0;

    frustum fr;
    frustum_from_matrix(&fr, vp);

    hm_iter it;
    uint64_t key;
    void *val;
    hm_iter_init(&it, &t->cache.map);
    while (hm_iter_next(&it, &key, &val)) {
        lt_cache_entry *e = val;
        if (!e->gpu.uploaded || e->gpu.index_count == 0) continue;

        // skip rings that are entirely offscreen. the far rings are exactly the
        // ones the frustum tends to clip, so this pays for itself.
        if (!frustum_contains_aabb(&fr, chunk_box(e->cx, e->cz))) continue;

        // hand the shader this chunk's morph weight so it can ease the level
        // transition (cross-dim / vertical bias). 1.0 when settled.
        gl_set_uniform_float(shader, "u_lod_morph", lt_morph_weight(&e->morph));

        lt_gpu_draw(&e->gpu);
        t->stats.drawn++;
        t->stats.tris_drawn += e->tri_count;
    }
}
