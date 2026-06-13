#include "biome_lookup.h"
#include "biome_table.h"

#include <float.h>
#include <stddef.h>

// oceans are special-cased a bit: erosion near 1.0 means "drowned lowland".
// the envelope weights already lean that way but we give a small extra nudge
// so coastlines read as ocean->beach->land instead of mushy gradients.

float biome_lookup_dist2(const biome_climate *c, const biome_def *def) {
    const biome_envelope *e = &def->env;
    float dt = c->temperature - e->temperature;
    float dh = c->humidity    - e->humidity;
    float de = c->erosion     - e->erosion;
    float dw = c->weirdness   - e->weirdness;
    return e->w_temp  * dt * dt
         + e->w_humid * dh * dh
         + e->w_eros  * de * de
         + e->w_weird * dw * dw;
}

biome_kind biome_lookup_pick_scored(const biome_climate *c, float *out_score) {
    biome_kind best = BIOME_KIND_PLAINS;
    float best_d = FLT_MAX;
    int n = biome_table_count();
    for (int i = 0; i < n; i++) {
        const biome_def *d = biome_table_at(i);
        float dist = biome_lookup_dist2(c, d);
        if (dist < best_d) {
            best_d = dist;
            best = d->kind;
        }
    }
    if (out_score) *out_score = best_d;
    return best;
}

biome_kind biome_lookup_pick(const biome_climate *c) {
    return biome_lookup_pick_scored(c, NULL);
}

// tiny insertion-sorted top-k. k is small (<=4 in practice) so o(n*k) is fine
// and beats dragging in a heap.
int biome_lookup_topk(const biome_climate *c, int k,
                      biome_kind *ids, float *dist2) {
    if (k < 1) return 0;
    int filled = 0;
    int n = biome_table_count();

    for (int i = 0; i < n; i++) {
        const biome_def *d = biome_table_at(i);
        float dist = biome_lookup_dist2(c, d);

        // skip if it cant beat the current worst and we're full
        if (filled == k && dist >= dist2[filled - 1]) continue;

        // find insertion slot
        int pos = filled < k ? filled : k - 1;
        while (pos > 0 && dist2[pos - 1] > dist) {
            // shift the loser down one
            dist2[pos] = dist2[pos - 1];
            ids[pos]   = ids[pos - 1];
            pos--;
        }
        dist2[pos] = dist;
        ids[pos]   = d->kind;
        if (filled < k) filled++;
    }
    return filled;
}
