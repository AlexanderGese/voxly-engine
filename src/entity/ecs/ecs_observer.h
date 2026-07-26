#ifndef ENTITY_ECS_OBSERVER_H
#define ENTITY_ECS_OBSERVER_H
#include "ecs_world.h"
#include "ecs_components.h"
typedef enum {
    ECS_HOOK_ADD = 0,
    ECS_HOOK_REMOVE
} ecs_hook_kind;
#define ECS_MAX_OBSERVERS  32
typedef struct {
    ecs_observer_fn fn;
    void           *user;
    ecs_cmp         cmp;
    ecs_hook_kind   kind;
    int             active;
} ecs_observer;
#endif
