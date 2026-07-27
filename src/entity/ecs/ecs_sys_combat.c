#include "ecs_sys_combat.h"
#include "ecs_components.h"
#include "../../math/vec3.h"
if (amount < 0.0f) amount = 0.0f;
ecs_events_emit(c->events, ECS_EV_DAMAGED, src, dst, amount, VEC3_ZERO);
ecs_combat_ctx *c = user;
