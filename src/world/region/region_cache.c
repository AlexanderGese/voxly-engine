#include "region_cache.h"
#include "region_coord.h"
#include "../../util/log.h"

#include <string.h>

void region_cache_init(region_cache_t *cache, const char *dir) {
    memset(cache, 0, sizeof *cache);
    snprintf(cache->dir, sizeof cache->dir, "%s", dir);
    hashmap_init(&cache->index, 16);
    cache->clock = 1;
}

static void evict_node(region_cache_t *cache, region_cache_node *n) {
    if (!n->valid) return;
    hashmap_del(&cache->index, n->key);
    region_file_close(&n->rf);
    n->valid = 0;
    n->key = 0;
    n->used = 0;
}

void region_cache_shutdown(region_cache_t *cache) {
    for (int i = 0; i < REGION_CACHE_MAX; i++) evict_node(cache, &cache->nodes[i]);
    hashmap_free(&cache->index);
}

void region_cache_flush(region_cache_t *cache) {
    for (int i = 0; i < REGION_CACHE_MAX; i++) {
        region_cache_node *n = &cache->nodes[i];
        if (n->valid && n->rf.header.dirty)
            region_header_write(&n->rf.header, n->rf.f);
    }
}

// find a slot to put a new file in: prefer an empty one, else evict the lru.
static region_cache_node *pick_victim(region_cache_t *cache) {
    region_cache_node *lru = NULL;
    for (int i = 0; i < REGION_CACHE_MAX; i++) {
        region_cache_node *n = &cache->nodes[i];
        if (!n->valid) return n;
        if (!lru || n->used < lru->used) lru = n;
    }
    evict_node(cache, lru);
    return lru;
}

region_file_t *region_cache_acquire(region_cache_t *cache, int cx, int cz, int create) {
    int rx = region_chunk_to_rx(cx);
    int rz = region_chunk_to_rz(cz);
    uint64_t key = region_key(rx, rz);

    region_cache_node *hit = hashmap_get(&cache->index, key);
    if (hit) {
        hit->used = ++cache->clock;
        return &hit->rf;
    }

    region_cache_node *n = pick_victim(cache);
    if (region_file_open(&n->rf, cache->dir, rx, rz, create) != 0) {
        // missing file with create==0, or io error. leave the slot empty.
        n->valid = 0;
        return NULL;
    }
    n->key   = key;
    n->valid = 1;
    n->used  = ++cache->clock;
    hashmap_put(&cache->index, key, n);
    return &n->rf;
}
