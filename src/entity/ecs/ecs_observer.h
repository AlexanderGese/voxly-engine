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
// here instead, which wrap them and fire the hooks. the stock systems use the
// wrappers; anything perf-critical that doesnt care about hooks can still hit
// the raw world functions.

typedef enum {
    ECS_HOOK_ADD = 0,
    ECS_HOOK_REMOVE
} ecs_hook_kind;

// fired with the entity, the component slot pointer (valid for ADD, already
// detached but still readable for REMOVE), and the user ctx you registered.
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

// register a hook. returns its slot index, or -1 if the table is full.
int  ecs_observe(ecs_observers *o, ecs_cmp c, ecs_hook_kind kind,
                 ecs_observer_fn fn, void *user);

// tear a hook back out by the index ecs_observe handed you.
void ecs_unobserve(ecs_observers *o, int handle);

// hook-firing wrappers around the raw world ops. add returns the new slot,
// remove returns nothing. both fire any matching observers.
void *ecs_obs_add(ecs_world *w, ecs_observers *o, ecs_entity e, ecs_cmp c,
                  const void *data);
void  ecs_obs_remove(ecs_world *w, ecs_observers *o, ecs_entity e, ecs_cmp c);

// fire hooks manually -- the deferred destroy path in the world cant call the
// wrappers, so the tick driver calls this for each component of a dying entity.
void  ecs_obs_fire(ecs_world *w, ecs_observers *o, ecs_entity e, ecs_cmp c,
                   ecs_hook_kind kind);

#endif
