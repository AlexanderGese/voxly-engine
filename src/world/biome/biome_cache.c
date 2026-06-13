#include "biome_cache.h"
#include "biome_noise.h"

#include <string.h>

// hash the column key into a table index. reuse the biome hash so we get a
// decent spread without inventing another mixer.
static uint32_t key_index(int wx, int wz, uint32_t seed) {
    return biome_hash2(wx, wz, seed) & BIOME_CACHE_MASK;
}

void biome_cache_init(biome_cache *cache, uint32_t seed, int sea_level, int radius) {
    if (!cache) return;
    memset(cache, 0, sizeof *cache);
    cache->seed = seed;
    cache->sea_level = sea_level;
    cache->radius = radius < 0 ? 0 : radius;
}

void biome_cache_clear(biome_cache *cache) {
    if (!cache) return;
    for (uint32_t i = 0; i < BIOME_CACHE_SIZE; i++) cache->slots[i].used = 0;
    cache->tick = 0;
    cache->hits = cache->misses = 0;
}

const biome_column *biome_cache_get(biome_cache *cache, int wx, int wz) {
    if (!cache) return NULL;
    cache->tick++;

    uint32_t idx = key_index(wx, wz, cache->seed);

    // linear probe looking for a hit
    uint32_t i = idx;
    for (int p = 0; p < BIOME_CACHE_PROBE; p++) {
        biome_cache_slot *s = &cache->slots[i];
        if (s->used && s->wx == wx && s->wz == wz) {
            s->stamp = cache->tick;
            cache->hits++;
            return &s->col;
        }
        if (!s->used) break;   // empty slot in the run -> definitely a miss
        i = (i + 1) & BIOME_CACHE_MASK;
    }

    cache->misses++;

    // miss: find a victim. prefer an empty slot in the probe window, else evict
    // the oldest one we saw (lowest stamp).
    uint32_t victim = idx;
    uint32_t oldest = 0xFFFFFFFFu;
    i = idx;
    for (int p = 0; p < BIOME_CACHE_PROBE; p++) {
        biome_cache_slot *s = &cache->slots[i];
        if (!s->used) { victim = i; oldest = 0; break; }
        if (s->stamp < oldest) { oldest = s->stamp; victim = i; }
        i = (i + 1) & BIOME_CACHE_MASK;
    }

    biome_cache_slot *vs = &cache->slots[victim];
    vs->used = 1;
    vs->wx = wx;
    vs->wz = wz;
    vs->stamp = cache->tick;
    biome_column_build(&vs->col, wx, wz, cache->sea_level, cache->seed, cache->radius);
    return &vs->col;
}

biome_kind biome_cache_biome(biome_cache *cache, int wx, int wz) {
    const biome_column *col = biome_cache_get(cache, wx, wz);
    return col ? col->biome : BIOME_KIND_PLAINS;
}

float biome_cache_hitrate(const biome_cache *cache) {
    if (!cache) return 0.0f;
    uint64_t total = cache->hits + cache->misses;
    if (total == 0) return 0.0f;
    return (float)cache->hits / (float)total * 100.0f;
}
