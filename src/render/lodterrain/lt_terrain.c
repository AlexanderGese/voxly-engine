#include "lt_terrain.h"
#include "lt_build.h"
#include "lt_mesh.h"
#include "lt_seam.h"
#include "lt_neighbor.h"
#include "../../math/aabb.h"
#include "../../config.h"
#include <stddef.h>
static block_id src_sample(void *ctx, int wx, int wy, int wz) {
    world *w = ctx;
    if (wy < 0 || wy >= CHUNK_SIZE_Y) return BLOCK_AIR;
    return world_get_block(w, wx, wy, wz);
}

static int src_light(void *ctx, int wx, int wy, int wz) {
    world *w = ctx;
if (wy < 0) return 0;
if (wy >= CHUNK_SIZE_Y) return MAX_LIGHT;
int bl = world_get_blocklight(w, wx, wy, wz);
int sl = world_get_sunlight(w, wx, wy, wz);
return bl > sl ? bl : sl;
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
out->level[1] = world_get_chunk(w, cx - 1, cz) ? level_for(t, cx - 1, cz, cam) : -1;
out->level[2] = world_get_chunk(w, cx, cz + 1) ? level_for(t, cx, cz + 1, cam) : -1;
out->level[3] = world_get_chunk(w, cx, cz - 1) ? level_for(t, cx, cz - 1, cam) : -1;
}

// per-chunk visit context. world_visit hands us one chunk at a time.
typedef struct {
    lt_terrain *t;
    world      *w;
    vec3        cam;
    float       dt;
} update_ctx;
t->stats.chunks_considered = 0;
t->stats.builds_this_frame = 0;
t->stats.builds_deferred   = 0;
;
world_visit(w, visit_select, &uc);
hm_iter it;
uint64_t key;
void *val;
hm_iter_init(&it, &t->cache.map);
int to_evict_cx[64];
int to_evict_cz[64];
int n_evict = 0;
i < n_evict;
i++)
        lt_cache_evict(&t->cache, to_evict_cx[i], to_evict_cz[i]);
t->stats.drawn = 0;
t->stats.tris_drawn = 0;
frustum fr;
frustum_from_matrix(&fr, vp);
hm_iter it;
uint64_t key;
void *val;
hm_iter_init(&it, &t->cache.map);
}
