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
i < REGION_CACHE_MAX;
for (int i = 0;
i < REGION_CACHE_MAX;
return lru;
