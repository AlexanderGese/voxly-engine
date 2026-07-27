#ifndef ENTITY_ECS_STATS_H
#define ENTITY_ECS_STATS_H

#include <stdint.h>

#include "ecs_world.h"
#include "ecs_components.h"
#include "ecs_system.h"
#include "ecs_events.h"

// the numbers behind the F3-style debug overlay. nothing here changes gameplay;
// it samples the ecs each frame so the hud can draw entity counts, per-component
// population, the busiest system, and a smoothed tick time. kept out of the hot
// systems so it can be compiled out (or just not called) without touching them.

typedef struct {
    uint32_t live_entities;
    uint32_t per_component[ECS_CMP_COUNT];  // how many entities own each cmp
    uint32_t free_slots;                    // recycled slots waiting in the pool

    double   tick_ms;          // last full scheduler tick, smoothed
    double   tick_ms_peak;     // worst tick since reset
    const char *hottest_sys;   // name of the slowest system last tick
    double   hottest_ms;

    uint64_t events_total;
    uint64_t events_dropped;

    double   ema_alpha;        // smoothing factor for tick_ms (0..1)
} ecs_stats;

void ecs_stats_init(ecs_stats *s);
void ecs_stats_reset_peaks(ecs_stats *s);

// sample everything for one frame. call after ecs_tick so the system timings are
// fresh. `events` may be NULL if you arent running the event bus.
void ecs_stats_sample(ecs_stats *s, const ecs_world *w,
                      const ecs_scheduler *sched, const ecs_events *events);

// format a compact multi-line summary into `buf` for the overlay. returns the
// length written (excluding the nul), clamped to cap-1.
int  ecs_stats_format(const ecs_stats *s, char *buf, int cap);

#endif
