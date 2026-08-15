#include "xp_event.h"

#include <stddef.h>
#include <string.h>

void xp_event_init(xp_event_log *l) {
    memset(l, 0, sizeof *l);
    l->head = 0;
    l->count = 0;
    l->seq = 0;
}

void xp_event_push(xp_event_log *l, xp_event_kind kind, int amount, vec3 where) {
    xp_event *e = &l->ring[l->head];
    e->kind = kind;
    e->amount = amount;
    e->where = where;
    e->age = 0.0f;

    l->head = (l->head + 1) % XP_EVENT_RING;
    if (l->count < XP_EVENT_RING) l->count++;
    l->seq++;
}

int xp_event_tick(xp_event_log *l, float dt, float max_age) {
    // age everything. we don't actually remove from the ring (it's circular),
    // we just let `count` shrink once the oldest entries expire. since pushes
    // are time-ordered, the tail is always the oldest.
    int live = 0;
    for (int i = 0; i < l->count; i++) {
        int idx = (l->head - 1 - i + XP_EVENT_RING * 2) % XP_EVENT_RING;
        xp_event *e = &l->ring[idx];
        e->age += dt;
        if (e->age <= max_age) live++;
    }
    // trim count to the number of non-expired entries (they're contiguous at
    // the front since order is preserved).
    if (live < l->count) l->count = live;
    return live;
}

const xp_event *xp_event_recent(const xp_event_log *l, int i) {
    if (i < 0 || i >= l->count) return NULL;
    int idx = (l->head - 1 - i + XP_EVENT_RING * 2) % XP_EVENT_RING;
    return &l->ring[idx];
}
