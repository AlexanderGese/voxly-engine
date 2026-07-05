#include "decals_cull.h"
#include "decals_config.h"

#include <string.h>

// pack a sort key: priority in the high bits (higher = later), then a phase
// rank so dying decals sort under live ones at equal priority, then a coarse
// distance bucket so nearer decals win ties (less z-fight on overlap). all
// ascending: smaller key draws first.
static uint32_t make_sort_key(const decals_decal *d, float dist_sq) {
    uint32_t prio  = (uint32_t)d->priority & 0xffffu;          // 16 bits
    uint32_t phase = (d->phase == DECALS_PHASE_DYING) ? 0u : 1u; // 1 bit
    // distance bucket: clamp sqrt-ish range into 14 bits. nearer = smaller, but
    // we invert so nearer draws *later* (on top) within a phase/priority tier.
    uint32_t db = (uint32_t)(dist_sq * 0.25f);
    if (db > 0x3fffu) db = 0x3fffu;
    db = 0x3fffu - db;
    return (prio << 15) | (phase << 14) | db;
}

// straight insertion sort. the visible set is small (a few hundred at most)
// and nearly sorted frame to frame, so this beats qsort's overhead here.
static void sort_items(decals_cull_item *it, int n) {
    for (int i = 1; i < n; i++) {
        decals_cull_item key = it[i];
        int j = i - 1;
        while (j >= 0 && it[j].sort > key.sort) {
            it[j + 1] = it[j];
            j--;
        }
        it[j + 1] = key;
    }
}

void decals_cull_gather(decals_cull_result *out, const decals_pool *p,
                        mat4 vp, vec3 cam, float max_dist) {
    memset(out, 0, sizeof *out);
    frustum f;
    frustum_from_matrix(&f, vp);
    float max_dist_sq = max_dist * max_dist;

    for (int i = 0; i < DECALS_MAX; i++) {
        const decals_decal *d = &p->slots[i];
        if (!d->alive) continue;
        out->tested++;

        // fully transparent? nothing to draw, dont waste an instance slot.
        if (d->alpha <= 0.001f) { out->culled_alpha++; continue; }

        // distance cull, unless the decal explicitly opts out (signs, runes).
        vec3 to = vec3_sub(d->proj.center, cam);
        float dist_sq = vec3_length_sq(to);
        if (!(d->flags & DECALS_FLAG_WORLD_LOCKED) && dist_sq > max_dist_sq) {
            out->culled_dist++;
            continue;
        }

        // frustum cull the projector's world aabb. cheap and catches the bulk
        // of off-screen decals before we touch the instance buffer.
        if (!frustum_contains_aabb(&f, d->proj.bounds)) {
            out->culled_frustum++;
            continue;
        }

        decals_cull_item *item = &out->items[out->count++];
        item->slot    = (uint32_t)i;
        item->dist_sq = dist_sq;
        item->sort    = make_sort_key(d, dist_sq);
    }

    sort_items(out->items, out->count);
}
