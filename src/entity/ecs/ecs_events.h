#ifndef ENTITY_ECS_EVENTS_H
#define ENTITY_ECS_EVENTS_H

#include <stdint.h>

#include "ecs_types.h"
#include "../../math/vec3.h"

// tiny event bus glued onto the ecs. systems that need to talk to each other
// (the ai pass telling combat "this guy swung", the health pass telling loot
// "this guy died") shouldnt reach into each other's components and poke. they
// push an event, somebody else drains it next frame. keeps the system order
// honest and lets me unit test a system by feeding it a fake event stream.
//
// the queue is a fixed ring. if it overflows in a frame we drop the oldest and
// bump a counter -- losing a particle spawn event is fine, and a few hundred
// mobs never fill a 1024-slot ring anyway. double-buffered: producers write the
// back buffer, ecs_events_swap() flips it so consumers read a stable snapshot.

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

// push onto the back buffer. cheap, no allocation. returns 0 if it had to drop.
int  ecs_events_push(ecs_events *q, ecs_event ev);

// shorthands for the common shapes so call sites stay terse.
int  ecs_events_emit(ecs_events *q, ecs_event_kind k, ecs_entity src,
                     ecs_entity dst, float amount, vec3 where);

// flip the buffers. call once per frame after producers, before consumers.
void ecs_events_swap(ecs_events *q);

// readable snapshot accessors -- consumers walk these over the front buffer.
uint32_t         ecs_events_count(const ecs_events *q);
const ecs_event *ecs_events_at(const ecs_events *q, uint32_t i);

// filter helper: copies matching events into `out` (cap `out_cap`), returns the
// number written. pass ECS_EV_NONE to match any kind.
uint32_t ecs_events_collect(const ecs_events *q, ecs_event_kind kind,
                            ecs_event *out, uint32_t out_cap);

#endif
