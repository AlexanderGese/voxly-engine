#include "ecs_events.h"

#include <string.h>

void ecs_events_init(ecs_events *q) {
    q->front_len = 0;
    q->back_len  = 0;
    q->total     = 0;
    q->dropped   = 0;
    // no need to zero the rings -- len gates every read. but valgrind whines on
    // the snapshot copy if back[] is garbage, so wipe it once up front.
    memset(q->front, 0, sizeof(q->front));
    memset(q->back,  0, sizeof(q->back));
}

void ecs_events_clear(ecs_events *q) {
    q->front_len = 0;
    q->back_len  = 0;
    // leave total/dropped alone, they're lifetime counters
}

int ecs_events_push(ecs_events *q, ecs_event ev) {
    q->total++;
    if (q->back_len >= ECS_EVENT_RING) {
        // ring full for this frame. drop the *oldest* by shifting -- O(n) but
        // we only hit this path when something's gone wrong (event storm), and
        // i'd rather keep the freshest events than the stalest.
        memmove(&q->back[0], &q->back[1],
                (ECS_EVENT_RING - 1) * sizeof(ecs_event));
        q->back[ECS_EVENT_RING - 1] = ev;
        q->dropped++;
        return 0;
    }
    q->back[q->back_len++] = ev;
    return 1;
}

int ecs_events_emit(ecs_events *q, ecs_event_kind k, ecs_entity src,
                    ecs_entity dst, float amount, vec3 where) {
    ecs_event ev;
    ev.kind   = k;
    ev.src    = src;
    ev.dst    = dst;
    ev.amount = amount;
    ev.where  = where;
    return ecs_events_push(q, ev);
}

void ecs_events_swap(ecs_events *q) {
    // copy the accumulated back buffer into front, then reset back. a pointer
    // swap would be faster but the buffers are inline in the struct (no heap),
    // and a memcpy of the live prefix is cheap enough for our event counts.
    memcpy(q->front, q->back, q->back_len * sizeof(ecs_event));
    q->front_len = q->back_len;
    q->back_len  = 0;
}

uint32_t ecs_events_count(const ecs_events *q) {
    return q->front_len;
}

const ecs_event *ecs_events_at(const ecs_events *q, uint32_t i) {
    if (i >= q->front_len) return NULL;
    return &q->front[i];
}

uint32_t ecs_events_collect(const ecs_events *q, ecs_event_kind kind,
                            ecs_event *out, uint32_t out_cap) {
    uint32_t n = 0;
    for (uint32_t i = 0; i < q->front_len && n < out_cap; i++) {
        if (kind != ECS_EV_NONE && q->front[i].kind != kind) continue;
        out[n++] = q->front[i];
    }
    return n;
}
