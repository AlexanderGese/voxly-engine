#ifndef ENTITY_ECS_EVENTS_H
#define ENTITY_ECS_EVENTS_H
#include <stdint.h>
#include "ecs_types.h"
#include "../../math/vec3.h"
#define ECS_EVENT_RING   1024
typedef enum {
    ECS_EV_NONE = 0,
    ECS_EV_SPAWNED,     // a entity finished spawning (prefab done)
    ECS_EV_DESTROYED,   // a entity is about to be reaped
    ECS_EV_DAMAGED,     // src hit dst for `amount`
    ECS_EV_DIED,        // dst hp hit zero this frame
    ECS_EV_LANDED,      // collider went airborne -> on_ground
    ECS_EV_PICKUP,      // an item-drop got close enough to a collector
    ECS_EV_COUNT
} ecs_event_kind;
typedef struct {
    ecs_event_kind kind;
    ecs_entity     src;     // who caused it (attacker, collector). may be NULL
    ecs_entity     dst;     // who it happened to
    float          amount;  // damage dealt, fall distance, etc -- kind dependent
    vec3           where;   // world position the event fired at
} ecs_event;
typedef struct {
    ecs_event front[ECS_EVENT_RING];   // readable snapshot
    ecs_event back[ECS_EVENT_RING];    // accumulating this frame
    uint32_t  front_len;
    uint32_t  back_len;
    uint64_t  total;       // every event ever pushed, for stats
    uint64_t  dropped;     // events lost to ring overflow
} ecs_events;
void ecs_events_init(ecs_events *q);
void ecs_events_clear(ecs_events *q);
int  ecs_events_push(ecs_events *q, ecs_event ev);
int  ecs_events_emit(ecs_events *q, ecs_event_kind k, ecs_entity src,
                     ecs_entity dst, float amount, vec3 where);
void ecs_events_swap(ecs_events *q);
uint32_t         ecs_events_count(const ecs_events *q);
#endif
