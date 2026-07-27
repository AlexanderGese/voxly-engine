#ifndef ENTITY_ECS_SYS_MOVEMENT_H
#define ENTITY_ECS_SYS_MOVEMENT_H

#include "ecs_world.h"

// movement + lifetime systems. these are the bread-and-butter ticks that run on
// the ecs every frame. they're plain ecs_system_fn callbacks so you just hand
// them to ecs_register with the order you want.

// integrates velocity into transform, applies gravity to anything with a
// collider, and clamps to terminal velocity. user ctx is unused (pass NULL).
void ecs_sys_movement(ecs_world *w, float dt, void *user);

// counts down ecs_lifetime and queues a destroy when it hits zero. order this
// late so a thing that just spawned this frame doesnt vanish before anyone
// sees it.
void ecs_sys_lifetime(ecs_world *w, float dt, void *user);

// ticks health hurt-timers down and reaps anything at <=0 hp. emits nothing,
// just destroys -- death drops / xp are handled elsewhere off a callback we
// havent wired through the ecs yet.
void ecs_sys_health(ecs_world *w, float dt, void *user);

#endif
