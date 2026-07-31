#include "navmesh.h"
#include "nav_query.h"
#include "../../config.h"
#include "../../util/log.h"

#include <math.h>
#include <string.h>

// patches tile the world on a NAV_PATCH_CHUNKS grid. a block's patch address
// is its chunk coord floored to that grid. floor-divide because chunk coords
// go negative and C truncates toward zero, which would mis-bucket -1.
static int floordiv(int a, int b) {
    int q = a / b;
    if ((a % b != 0) && ((a < 0) != (b < 0))) q--;
    return q;
}

void navmesh_patch_addr(int wx, int wz, int *pcx, int *pcz) {
    int cx = floordiv(wx, CHUNK_SIZE_X);
    int cz = floordiv(wz, CHUNK_SIZE_Z);
    *pcx = floordiv(cx, NAV_PATCH_CHUNKS) * NAV_PATCH_CHUNKS;
    *pcz = floordiv(cz, NAV_PATCH_CHUNKS) * NAV_PATCH_CHUNKS;
}

void navmesh_init(navmesh *nm, world *w) {
    nm->w      = w;
    nm->clock  = 1;
    nm->builds = 0;
    memset(nm->patches, 0, sizeof nm->patches);
    for (int i = 0; i < NAV_CACHE_PATCHES; i++)
        nav_grid_init(&nm->patches[i].grid);
    hashmap_init(&nm->addr, 32);
}

void navmesh_free(navmesh *nm) {
    for (int i = 0; i < NAV_CACHE_PATCHES; i++)
        nav_grid_free(&nm->patches[i].grid);
    hashmap_free(&nm->addr);
    nm->w = NULL;
}

void navmesh_invalidate_all(navmesh *nm) {
    for (int i = 0; i < NAV_CACHE_PATCHES; i++)
        nm->patches[i].valid = 0;
    hashmap_free(&nm->addr);
    hashmap_init(&nm->addr, 32);
}

void navmesh_invalidate_at(navmesh *nm, int wx, int wz) {
    int pcx, pcz;
    navmesh_patch_addr(wx, wz, &pcx, &pcz);
    uint64_t key = hm_pack2(pcx, pcz);
    void *v = hashmap_get(&nm->addr, key);
    if (!v) return;
    int slot = (int)((intptr_t)v) - 1;
    nm->patches[slot].valid = 0;
    hashmap_del(&nm->addr, key);
}

// pick a slot to bake into: first empty, else the least-recently-used one.
static int pick_slot(navmesh *nm) {
    int lru = 0;
    uint32_t best = 0xffffffffu;
    for (int i = 0; i < NAV_CACHE_PATCHES; i++) {
        if (!nm->patches[i].valid) return i;
        if (nm->patches[i].stamp < best) {
            best = nm->patches[i].stamp;
            lru = i;
        }
    }
    return lru;
}

// find the cached slot for a patch addr, or bake it. returns slot index.
static int get_patch(navmesh *nm, int pcx, int pcz) {
    uint64_t key = hm_pack2(pcx, pcz);
    void *v = hashmap_get(&nm->addr, key);
    if (v) {
        int slot = (int)((intptr_t)v) - 1;
        if (nm->patches[slot].valid &&
            nm->patches[slot].pcx == pcx && nm->patches[slot].pcz == pcz) {
            nm->patches[slot].stamp = nm->clock++;   // touch for lru
            return slot;
        }
        // stale map entry, fall through and rebake.
        hashmap_del(&nm->addr, key);
    }

    int slot = pick_slot(nm);
    nav_patch *p = &nm->patches[slot];

    // if we're evicting a live patch, drop its old addr mapping first.
    if (p->valid) {
        hashmap_del(&nm->addr, hm_pack2(p->pcx, p->pcz));
    }

    nav_build_stats st;
    nav_build_patch(&p->grid, nm->w, pcx, pcz, &st);
    p->pcx   = pcx;
    p->pcz   = pcz;
    p->valid = 1;
    p->stamp = nm->clock++;
    nm->builds++;

    hashmap_put(&nm->addr, key, (void*)(intptr_t)(slot + 1));

    if (st.partial)
        LOGW("navmesh: patch %d,%d hit cell cap (%d cells, %d regions)",
             pcx, pcz, st.cells, st.regions);
    else
        LOGD("navmesh: baked patch %d,%d -> %d cells, %d regions",
             pcx, pcz, st.cells, st.regions);

    return slot;
}

nav_grid *navmesh_grid_for(navmesh *nm, vec3 p) {
    int wx = (int)floorf(p.x);
    int wz = (int)floorf(p.z);
    int pcx, pcz;
    navmesh_patch_addr(wx, wz, &pcx, &pcz);
    int slot = get_patch(nm, pcx, pcz);
    nav_grid *g = &nm->patches[slot].grid;
    return g->count > 0 ? g : NULL;
}

int navmesh_cell_at(navmesh *nm, vec3 p, nav_grid **g_out) {
    nav_grid *g = navmesh_grid_for(nm, p);
    if (g_out) *g_out = g;
    if (!g) return -1;
    // small radius search so an entity standing right on a patch seam still
    // resolves; the apron means the floor exists in the neighbour bake too.
    return nav_query_nearest(g, p, 2);
}
