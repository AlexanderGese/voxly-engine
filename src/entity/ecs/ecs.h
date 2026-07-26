#ifndef ENTITY_ECS_H
#define ENTITY_ECS_H

// umbrella header. pull this in and you get the whole little ecs: world,
// queries, systems, the stock systems, and prefabs. individual headers exist
// too if you only need a slice and want to keep your include graph honest.

#include "ecs_types.h"
#include "ecs_components.h"
#include "ecs_world.h"
#include "ecs_query.h"
#include "ecs_system.h"
#include "ecs_prefab.h"
#include "ecs_snapshot.h"
#include "ecs_sys_movement.h"
#include "ecs_sys_ai.h"

// stock pipeline ordering. these are the orders the default systems register
// at; exposed so game code that adds its own systems can slot in between them
// (e.g. a knockback pass at order 5, after ai, before movement).
#define ECS_ORDER_AI         0
#define ECS_ORDER_MOVEMENT   10
#define ECS_ORDER_HEALTH     80
#define ECS_ORDER_LIFETIME   90

// registers the stock systems on `sched` against `world`. ai_ctx must outlive
// the scheduler since we only stash the pointer. pass NULL ai_ctx to skip the
// ai system (handy for the menu preview world that has no player to chase).
void ecs_register_default_systems(ecs_scheduler *sched, ecs_world *world,
                                  ecs_ai_ctx *ai_ctx);

#endif
