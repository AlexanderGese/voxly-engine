#include "ecs_system.h"

#include <string.h>

#include "../../util/timer.h"
#include "../../util/log.h"

void ecs_scheduler_init(ecs_scheduler *s) {
    s->count  = 0;
    s->sorted = 1;
}

int ecs_register(ecs_scheduler *s, const char *name, ecs_system_fn fn,
                 void *user, int order) {
    if (s->count >= ECS_MAX_SYSTEMS) {
        LOGE("ecs: system table full, dropping '%s'", name);
        return -1;
    }
    int i = s->count++;
    s->systems[i].name    = name;
    s->systems[i].fn      = fn;
    s->systems[i].user    = user;
    s->systems[i].order   = order;
    s->systems[i].enabled = 1;
    s->systems[i].last_ms = 0.0;
    s->sorted = 0;             // needs a re-sort before next tick
    return i;
}

void ecs_set_enabled(ecs_scheduler *s, const char *name, int enabled) {
    for (int i = 0; i < s->count; i++) {
        if (strcmp(s->systems[i].name, name) == 0) {
            s->systems[i].enabled = enabled;
            return;
        }
    }
    LOGW("ecs: set_enabled on unknown system '%s'", name);
}

// stable insertion sort by order. n is tiny (<=32) and runs at most once after
// registration churn, so anything cleverer would just be showing off.
static void sort_systems(ecs_scheduler *s) {
    for (int i = 1; i < s->count; i++) {
        ecs_system tmp = s->systems[i];
        int j = i - 1;
        while (j >= 0 && s->systems[j].order > tmp.order) {
            s->systems[j + 1] = s->systems[j];
            j--;
        }
        s->systems[j + 1] = tmp;
    }
    s->sorted = 1;
}

void ecs_tick(ecs_scheduler *s, ecs_world *w, float dt) {
    if (!s->sorted) sort_systems(s);

    // mark the world deferring so systems can safely ecs_destroy mid-iteration.
    w->deferring++;
    for (int i = 0; i < s->count; i++) {
        ecs_system *sys = &s->systems[i];
        if (!sys->enabled) { sys->last_ms = 0.0; continue; }

        uint64_t t0 = timer_now_us();
        sys->fn(w, dt, sys->user);
        sys->last_ms = timer_delta_s(t0) * 1000.0;
    }
    w->deferring--;

    // single flush for the whole frame -- batching the destroys means a store
    // only ever gets compacted once even if five systems killed entities.
    ecs_flush_destroyed(w);
}

double ecs_scheduler_last_total_ms(const ecs_scheduler *s) {
    double total = 0.0;
    for (int i = 0; i < s->count; i++)
        total += s->systems[i].last_ms;
    return total;
}
