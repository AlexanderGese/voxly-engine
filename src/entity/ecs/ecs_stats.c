#include "ecs_stats.h"
#include "ecs_store.h"

#include <stdio.h>
#include <string.h>

void ecs_stats_init(ecs_stats *s) {
    memset(s, 0, sizeof *s);
    s->hottest_sys = "-";
    s->ema_alpha   = 0.2f;   // ~5 frame smoothing, enough to stop the jitter
}

void ecs_stats_reset_peaks(ecs_stats *s) {
    s->tick_ms_peak = 0.0;
    s->hottest_ms   = 0.0;
}

void ecs_stats_sample(ecs_stats *s, const ecs_world *w,
                      const ecs_scheduler *sched, const ecs_events *events) {
    s->live_entities = ecs_count(w);

    // per-component population comes straight off each store's dense length --
    // the dense array is exactly the set of entities owning that component.
    for (int c = 0; c < ECS_CMP_COUNT; c++)
        s->per_component[c] = ecs_store_count(&w->stores[c]);

    // slots sitting in the freelist are recycled-and-waiting; hiwater - live -
    // free should reconcile, but we only surface the freelist depth here.
    s->free_slots = w->pool.free_len;

    // walk the scheduler for the slowest system + total tick time. the systems
    // each stamped last_ms during ecs_tick, so this is just a reduction.
    double total = 0.0;
    double worst = 0.0;
    const char *worst_name = "-";
    for (int i = 0; i < sched->count; i++) {
        const ecs_system *sys = &sched->systems[i];
        if (!sys->enabled) continue;
        total += sys->last_ms;
        if (sys->last_ms > worst) {
            worst      = sys->last_ms;
            worst_name = sys->name;
        }
    }

    // exponential moving average on the tick time so the hud number sits still
    // enough to read. first sample seeds it directly instead of crawling up.
    if (s->tick_ms <= 0.0) s->tick_ms = total;
    else s->tick_ms += s->ema_alpha * (total - s->tick_ms);

    if (total > s->tick_ms_peak) s->tick_ms_peak = total;
    s->hottest_sys = worst_name;
    s->hottest_ms  = worst;
    if (worst > s->hottest_ms) s->hottest_ms = worst;

    if (events) {
        s->events_total   = events->total;
        s->events_dropped = events->dropped;
    }
}

int ecs_stats_format(const ecs_stats *s, char *buf, int cap) {
    if (cap <= 0) return 0;
    // compact, one fact per line. snprintf clamps for us; we just track the cut.
    int n = snprintf(buf, (size_t)cap,
        "ecs: %u ent  (%u free)\n"
        "tick %.2fms (peak %.2f)\n"
        "hot: %s %.2fms\n"
        "tf:%u vel:%u col:%u hp:%u ai:%u\n"
        "events %llu (dropped %llu)",
        s->live_entities, s->free_slots,
        s->tick_ms, s->tick_ms_peak,
        s->hottest_sys ? s->hottest_sys : "-", s->hottest_ms,
        s->per_component[ECS_CMP_TRANSFORM],
        s->per_component[ECS_CMP_VELOCITY],
        s->per_component[ECS_CMP_COLLIDER],
        s->per_component[ECS_CMP_HEALTH],
        s->per_component[ECS_CMP_AI],
        (unsigned long long)s->events_total,
        (unsigned long long)s->events_dropped);

    if (n < 0) { buf[0] = '\0'; return 0; }
    if (n >= cap) n = cap - 1;     // snprintf returns would-be length
    return n;
}
