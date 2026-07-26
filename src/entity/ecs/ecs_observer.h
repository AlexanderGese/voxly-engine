#ifndef ENTITY_ECS_OBSERVER_H
#define ENTITY_ECS_OBSERVER_H
#include "ecs_world.h"
#include "ecs_components.h"
// component lifecycle hooks. you register a callback against a component id and
// it fires when that component gets added to or removed from an entity. i added
// this because half the systems wanted to do setup/teardown the instant a
// component appeared (allocate a render handle when ECS_CMP_RENDERABLE shows up,
// free it when it goes) and threading that through every ecs_add call site by
// hand was getting silly.
//
// note: the plain ecs_add/ecs_remove in ecs_world.c dont call these -- they
// cant without an include cycle. you go through ecs_obs_add / ecs_obs_remove
typedef enum {
    ECS_HOOK_ADD = 0,
    ECS_HOOK_REMOVE
} ecs_hook_kind;
typedef void (*ecs_observer_fn)(ecs_world *w, ecs_entity e, ecs_cmp c,
                                void *slot, void *user);
#define ECS_MAX_OBSERVERS  32
typedef struct {
    ecs_observer_fn fn;
    void           *user;
    ecs_cmp         cmp;
    ecs_hook_kind   kind;
    int             active;
} ecs_observer;
typedef struct {
    ecs_observer obs[ECS_MAX_OBSERVERS];
    int          count;
} ecs_observers;
void ecs_observers_init(ecs_observers *o);
int  ecs_observe(ecs_observers *o, ecs_cmp c, ecs_hook_kind kind,
                 ecs_observer_fn fn, void *user);
void ecs_unobserve(ecs_observers *o, int handle);
void *ecs_obs_add(ecs_world *w, ecs_observers *o, ecs_entity e, ecs_cmp c,
                  const void *data);
void  ecs_obs_remove(ecs_world *w, ecs_observers *o, ecs_entity e, ecs_cmp c);
void  ecs_obs_fire(ecs_world *w, ecs_observers *o, ecs_entity e, ecs_cmp c,
                   ecs_hook_kind kind);
#endif
