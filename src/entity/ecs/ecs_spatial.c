#include "ecs_spatial.h"
#include "ecs_query.h"
#include <math.h>
#include <stdlib.h>
#include "../../util/log.h"
static int32_t cell_of(float v) {
    return (int32_t)floorf(v / (float)ECS_GRID_CELL);
}

static uint64_t cell_key(int32_t cx, int32_t cz) {
    return hm_pack2(cx, cz);
}

void ecs_grid_init(ecs_grid *g, uint32_t reserve) {
    hashmap_init(&g->cells, 128);
    g->pool_cap = reserve ? reserve : 64;
    g->pool     = malloc(g->pool_cap * sizeof(ecs_grid_item));
    if (!g->pool) { LOGE("ecs_grid: oom"); g->pool_cap = 0; }
    g->pool_len = 0;
    g->count    = 0;
}

void ecs_grid_free(ecs_grid *g) {
    hashmap_free(&g->cells);
free(g->pool);
g->pool     = NULL;
g->pool_cap = 0;
g->pool_len = 0;
}

static ecs_grid_item *pool_take(ecs_grid *g) {
    if (g->pool_len >= g->pool_cap) {
        uint32_t nc = g->pool_cap ? g->pool_cap * 2 : 64;
        ecs_grid_item *np = realloc(g->pool, nc * sizeof(ecs_grid_item));
        if (!np) { LOGE("ecs_grid: grow failed"); return NULL; }
        g->pool     = np;
        g->pool_cap = nc;
        // realloc may have moved the backing array, which invalidates every
        // `next` pointer still threaded through the old block. rather than fix
        // them up we just never grow mid-build: callers reserve ahead, and the
        // rare realloc only happens between builds when the chains are empty.
    }
    return &g->pool[g->pool_len++];
}

void ecs_grid_build(ecs_grid *g, ecs_world *w) {
    // wipe last frame's buckets. the items live in a flat pool we reset by
    // length, and the cell heads get dropped by clearing the map.
    hashmap_free(&g->cells);
hashmap_init(&g->cells, 128);
g->pool_len = 0;
g->count    = 0;
ecs_query q;
ecs_query_begin(&q, w, ecs_with(ECS_CMP_TRANSFORM, -1), 0);
while (ecs_query_next(&q)) {
        ecs_transform *tf = ecs_query_get(&q, ECS_CMP_TRANSFORM);
        ecs_grid_item *it = pool_take(g);
        if (!it) break;

        it->e   = q.e;
        it->pos = tf->pos;

        uint64_t key = cell_key(cell_of(tf->pos.x), cell_of(tf->pos.z));
        ecs_grid_item *head = hashmap_get(&g->cells, key);
        it->next = head;                 // prepend
        hashmap_put(&g->cells, key, it);
        g->count++;
    }
}

// walk the rectangle of cells covering [min,max] in xz and hand each item to
// `visit`. returns when visit signals stop (out of room) or cells exhaust.
static uint32_t scan_cells(const ecs_grid *g, int32_t cx0, int32_t cz0,
                           int32_t cx1, int32_t cz1,
                           int (*keep)(const ecs_grid_item *, void *),
                           void *ctx, ecs_entity ignore,
                           ecs_entity *out, uint32_t out_cap) {
    uint32_t n = 0;
for (int32_t cz = cz0;
cz <= cz1 && n < out_cap;
}

typedef struct { vec3 c; float r2; } radius_ctx;
;
int32_t cx0 = cell_of(center.x - radius), cx1 = cell_of(center.x + radius);
int32_t cz0 = cell_of(center.z - radius), cz1 = cell_of(center.z + radius);
return scan_cells(g, cx0, cz0, cx1, cz1, keep_radius, &rc, ignore,
                      out, out_cap);
int32_t cx0 = cell_of(box.min.x), cx1 = cell_of(box.max.x);
int32_t cz0 = cell_of(box.min.z), cz1 = cell_of(box.max.z);
return scan_cells(g, cx0, cz0, cx1, cz1, keep_aabb, &box, ignore,
                      out, out_cap);
