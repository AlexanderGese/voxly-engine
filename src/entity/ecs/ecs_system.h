#ifndef ENTITY_ECS_SYSTEM_H
#define ENTITY_ECS_SYSTEM_H

#include "ecs_world.h"

// systems registry. a system is just a named callback run once per tick with
// the world and dt. they run in ascending `order` (ties keep insertion order),
// so movement (0) goes before collision (10) goes before despawn (90).
//
// nothing fancy -- no dependency graph, no parallel scheduling. we tick maybe a
// few hundred entities through eight systems; a flat ordered list is plenty and
// i can actually reason about it.

#define ECS_MAX_SYSTEMS  32

typedef void (*ecs_system_fn)(ecs_world *w, float dt, void *user);

typedef struct {
    const char    *name;
    ecs_system_fn  fn;
    void          *user;     // opaque ctx handed back to the callback
    int            order;
    int            enabled;
    double         last_ms;  // wall time the last run took, for the profiler hud
} ecs_system;

typedef struct {
    ecs_system systems[ECS_MAX_SYSTEMS];
    int        count;
    int        sorted;       // dirty flag; we re-sort lazily before a tick
} ecs_scheduler;

void ecs_scheduler_init(ecs_scheduler *s);

// register a system. lower order runs earlier. returns its index, or -1 if full.
int  ecs_register(ecs_scheduler *s, const char *name, ecs_system_fn fn,
                  void *user, int order);

void ecs_set_enabled(ecs_scheduler *s, const char *name, int enabled);

// run every enabled system in order, then flush deferred destroys exactly once.
void ecs_tick(ecs_scheduler *s, ecs_world *w, float dt);

// total of last_ms across systems -- handy for the debug overlay.
double ecs_scheduler_last_total_ms(const ecs_scheduler *s);

#endif
