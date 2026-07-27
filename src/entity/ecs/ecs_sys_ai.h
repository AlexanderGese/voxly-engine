#ifndef ENTITY_ECS_SYS_AI_H
#define ENTITY_ECS_SYS_AI_H

#include "ecs_world.h"
#include "../../math/vec3.h"

// the ecs brain tick. a cheap heuristic state machine driving mobs toward a
// target (usually the player). this is the component-array version of the older
// ai_tick(entity*) in ai.c -- same states, but it reads/writes ecs_ai and
// steers via ecs_velocity instead of mutating a fat entity struct.

// the system needs to know where the player is and how aggressive each mob
// should be, so it takes a context struct as its `user` pointer.
typedef struct {
    vec3  target_pos;     // where mobs chase toward / flee from
    float aggro_range;    // start chasing inside this radius
    float attack_range;   // switch to attack inside this
    float forget_range;   // give up chasing past this
    float move_speed;     // chase speed in blocks/sec
} ecs_ai_ctx;

void ecs_ai_ctx_defaults(ecs_ai_ctx *c, vec3 target_pos);

// register this with the scheduler, passing an ecs_ai_ctx* as user. it should
// run before movement so the velocity it sets gets integrated the same frame.
void ecs_sys_ai(ecs_world *w, float dt, void *user);

#endif
