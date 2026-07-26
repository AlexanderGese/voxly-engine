#include "ecs.h"

// just the glue that registers the stock pipeline in the right order. kept out
// of ecs_system.c so the scheduler stays ignorant of which concrete systems
// exist -- it only knows about callbacks and order numbers.

void ecs_register_default_systems(ecs_scheduler *sched, ecs_world *world,
                                  ecs_ai_ctx *ai_ctx) {
    (void)world;   // systems get the world at tick time, not registration

    if (ai_ctx) {
        ecs_register(sched, "ai", ecs_sys_ai, ai_ctx, ECS_ORDER_AI);
    }
    ecs_register(sched, "movement", ecs_sys_movement, NULL, ECS_ORDER_MOVEMENT);
    ecs_register(sched, "health",   ecs_sys_health,   NULL, ECS_ORDER_HEALTH);
    ecs_register(sched, "lifetime", ecs_sys_lifetime, NULL, ECS_ORDER_LIFETIME);
}
