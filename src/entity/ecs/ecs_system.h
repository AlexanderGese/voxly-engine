#ifndef ENTITY_ECS_SYSTEM_H
#define ENTITY_ECS_SYSTEM_H
#include "ecs_world.h"
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
#endif
