#include "lt_stats.h"
#include "lt_config.h"
#include <stdio.h>
#include <string.h>
void lt_stats_breakdown(const lt_cache *c, lt_level_breakdown *out) {
    memset(out, 0, sizeof(*out));

    hm_iter it;
    uint64_t key;
    void *val;
    hm_iter_init(&it, &c->map);
    while (hm_iter_next(&it, &key, &val)) {
        const lt_cache_entry *e = val;

        if (lt_cache_needs_build(e)) out->pending++;

        // not built yet -> nothing to tally per-level. skip until it commits.
        if (e->level < 0) continue;

        int lvl = e->level;
        if (lvl >= LT_LEVEL_COUNT) lvl = LT_LEVEL_COUNT - 1;

        out->chunks[lvl]++;
        out->tris[lvl] += e->tri_count;
        if (e->tri_count == 0) out->empty++;
    }
}

long lt_stats_mesh_bytes(int tris) {
    if (tris <= 0) return 0;
long quads = (tris + 1) / 2;
long verts = quads * 4;
long idx   = (long)tris * 3;
return verts * (long)sizeof(float[6]) + idx * 4;
}

long lt_stats_gpu_bytes(const lt_cache *c) {
    long total = 0;
    hm_iter it;
    uint64_t key;
    void *val;
    hm_iter_init(&it, &c->map);
    while (hm_iter_next(&it, &key, &val)) {
        const lt_cache_entry *e = val;
        total += lt_stats_mesh_bytes(e->tri_count);
    }
    return total;
}

float lt_stats_savings(const lt_cache *c) {
    // estimate what level 0 would have cost. coarser levels shrink geometry
    // by roughly the square of the linear step (a 2x coarser grid has ~1/4 the
    // surface cells), so scale each level's tris back up by step^2 to guess the
    // full-res cost, then compare.
    long actual = 0;
double full = 0.0;
hm_iter it;
uint64_t key;
void *val;
hm_iter_init(&it, &c->map);
while (hm_iter_next(&it, &key, &val)) {
        const lt_cache_entry *e = val;
        if (e->level < 0) continue;
        int step = LT_STEP(e->level);
        actual += e->tri_count;
        full   += (double)e->tri_count * (double)(step * step);
    }

    if (full <= 0.0) return 0.0f;
float saved = 1.0f - (float)((double)actual / full);
if (saved < 0.0f) saved = 0.0f;
if (saved > 1.0f) saved = 1.0f;
return saved;
