#include "ecs_stats.h"
#include "ecs_store.h"
#include <stdio.h>
#include <string.h>
s->hottest_ms   = 0.0;
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
