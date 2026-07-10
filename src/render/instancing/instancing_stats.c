#include "instancing_stats.h"

#include <stdio.h>
#include <string.h>

void instancing_stats_reset(instancing_stats *s) {
    memset(s, 0, sizeof *s);
}

void instancing_stats_add_batch(instancing_stats *s, int survivors,
                                int base_tris, int draw_calls) {
    if (survivors <= 0) return;
    s->batches++;
    s->instances_drawn += survivors;
    s->draw_calls += draw_calls;
    if (draw_calls > 1) s->batch_splits += (draw_calls - 1);
    s->tris_estimate += (long)survivors * (long)base_tris;
}

int instancing_stats_format(const instancing_stats *s, char *buf, int cap) {
    if (cap <= 0) return 0;
    // one tidy line for the debug overlay. the cull ratio is the headline
    // number — if it's near zero the frustum pass is basically free money.
    int culled = s->instances_total > 0 ? s->instances_culled : 0;
    int pct = s->instances_total > 0
                  ? (culled * 100) / s->instances_total
                  : 0;
    int n = snprintf(buf, (size_t)cap,
                     "inst: %d drawn / %d total (%d%% culled) | "
                     "%d batches, %d calls (+%d splits), ~%ldk tris",
                     s->instances_drawn, s->instances_total, pct,
                     s->batches, s->draw_calls, s->batch_splits,
                     s->tris_estimate / 1000);
    if (n < 0) return 0;
    if (n >= cap) n = cap - 1;
    return n;
}
