#include "lt_cache.h"

#include <stdlib.h>

void lt_cache_init(lt_cache *c) {
    hashmap_init(&c->map, 256);
    c->entry_count = 0;
    c->tri_total   = 0;
}

void lt_cache_free(lt_cache *c) {
    hm_iter it;
    uint64_t key;
    void *val;
    hm_iter_init(&it, &c->map);
    while (hm_iter_next(&it, &key, &val)) {
        lt_cache_entry *e = val;
        lt_gpu_free(&e->gpu);
        free(e);
    }
    hashmap_free(&c->map);
    c->entry_count = 0;
    c->tri_total   = 0;
}

lt_cache_entry *lt_cache_find(lt_cache *c, int cx, int cz) {
    return (lt_cache_entry*)hashmap_get(&c->map, hm_pack2(cx, cz));
}

lt_cache_entry *lt_cache_get(lt_cache *c, int cx, int cz) {
    uint64_t key = hm_pack2(cx, cz);
    lt_cache_entry *e = hashmap_get(&c->map, key);
    if (e) return e;

    e = calloc(1, sizeof(*e));
    if (!e) return NULL;   // oom; caller skips this chunk this frame

    e->cx = cx;
    e->cz = cz;
    e->level      = -1;     // nothing built yet
    e->want_level = -1;
    e->seam       = 0;
    e->want_seam  = 0;
    e->dirty      = 1;      // force a first build
    e->tri_count  = 0;
    lt_gpu_init(&e->gpu);
    lt_morph_init(&e->morph, 0, 3.0f);   // settles to its real level on commit

    hashmap_put(&c->map, key, e);
    c->entry_count++;
    return e;
}

void lt_cache_mark_dirty(lt_cache *c, int cx, int cz) {
    lt_cache_entry *e = lt_cache_find(c, cx, cz);
    if (e) e->dirty = 1;
}

void lt_cache_evict(lt_cache *c, int cx, int cz) {
    uint64_t key = hm_pack2(cx, cz);
    lt_cache_entry *e = hashmap_get(&c->map, key);
    if (!e) return;

    c->tri_total -= e->tri_count;
    if (c->tri_total < 0) c->tri_total = 0;

    lt_gpu_free(&e->gpu);
    free(e);
    hashmap_del(&c->map, key);
    c->entry_count--;
}

int lt_cache_needs_build(const lt_cache_entry *e) {
    if (e->dirty) return 1;
    if (e->level < 0) return 1;                 // never built
    if (e->level != e->want_level) return 1;    // selector wants a new level
    if (e->seam  != e->want_seam)  return 1;    // a neighbour relod'd
    return 0;
}

void lt_cache_commit(lt_cache *c, lt_cache_entry *e, int tris) {
    // roll the running triangle total: subtract the old, add the new.
    c->tri_total -= e->tri_count;
    e->tri_count = tris;
    c->tri_total += tris;
    if (c->tri_total < 0) c->tri_total = 0;

    e->level = e->want_level;
    e->seam  = e->want_seam;
    e->dirty = 0;
}
