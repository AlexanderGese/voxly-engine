#include "loader_debug.h"

#include <stdio.h>
#include "../../util/log.h"

void loader_debug_census(const loader *l, loader_ring_census *out, uint64_t now_us) {
    for (int i = 0; i < 6; i++) out->per_stage[i] = 0;
    out->occupied = out->resident = out->in_cooldown = 0;

    for (int i = 0; i < LOADER_RING_SLOTS; i++) {
        const loader_slot *s = &l->ring.slots[i];
        if (!s->occupied) continue;
        out->occupied++;
        // stage is in range by construction, but clamp anyway -- defensive against
        // a future stage being added without updating the array size.
        int st = (int)s->stage;
        if (st >= 0 && st < 6) out->per_stage[st]++;
        if (s->stage == LOADER_STAGE_RESIDENT) out->resident++;
        if (now_us < s->cooldown_us) out->in_cooldown++;
    }
}

int loader_debug_oneline(const loader *l, uint64_t now_us, char *buf, int cap) {
    loader_ring_census c;
    loader_debug_census(l, &c, now_us);

    // e.g. "chunks 142/169 res, q23, gen8 lit4 mesh2"
    int n = snprintf(buf, (size_t)cap,
                     "chunks %d/%d res, q%zu, g%d li%d me%d cd%d",
                     c.resident, c.occupied,
                     loader_pending(l),
                     c.per_stage[LOADER_STAGE_GENERATED],
                     c.per_stage[LOADER_STAGE_LIT],
                     c.per_stage[LOADER_STAGE_MESHED],
                     c.in_cooldown);
    if (n < 0) { if (cap > 0) buf[0] = '\0'; return 0; }
    if (n >= cap) n = cap - 1;   // snprintf truncated; report what fit
    return n;
}

void loader_debug_dump(const loader *l) {
    const loader_stats *st = &l->stats;
    LOGI("loader stats: alloc=%llu gen=%llu light=%llu mesh=%llu upload=%llu",
         (unsigned long long)st->ran[LOADER_JOB_ALLOC],
         (unsigned long long)st->ran[LOADER_JOB_GEN],
         (unsigned long long)st->ran[LOADER_JOB_LIGHT],
         (unsigned long long)st->ran[LOADER_JOB_MESH],
         (unsigned long long)st->ran[LOADER_JOB_UPLOAD]);
    LOGI("loader stats: retried=%llu failed=%llu stale=%llu pending=%zu",
         (unsigned long long)st->retried,
         (unsigned long long)st->failed,
         (unsigned long long)st->stale_dropped,
         loader_pending(l));
}
