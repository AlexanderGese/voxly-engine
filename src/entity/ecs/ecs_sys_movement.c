#include "ecs_sys_movement.h"
#include "ecs_query.h"

#include "../../config.h"

// semi-implicit euler. gravity first so the integrated position uses the
// post-gravity velocity -- feels less floaty than the other order, which i
// only noticed because jumps felt mushy for a week.
void ecs_sys_movement(ecs_world *w, float dt, void *user) {
    (void)user;
    ecs_query q;
    ecs_query_begin(&q, w,
                    ecs_with(ECS_CMP_TRANSFORM, ECS_CMP_VELOCITY, -1), 0);

    while (ecs_query_next(&q)) {
        ecs_transform *tf = ecs_query_get(&q, ECS_CMP_TRANSFORM);
        ecs_velocity  *vel = ecs_query_get(&q, ECS_CMP_VELOCITY);

        // gravity only applies to things that collide with the world. ghosts /
        // free-floating markers skip it.
        ecs_collider *col = ecs_get(w, q.e, ECS_CMP_COLLIDER);
        if (col && !col->on_ground) {
            vel->linear.y += GRAVITY * dt;
            if (vel->linear.y < TERMINAL_VELOCITY)
                vel->linear.y = TERMINAL_VELOCITY;
        }

        // per-second exponential-ish drag. cheap approximation, not a real
        // integral, but stable for the dt range we run at.
        if (vel->drag > 0.0f) {
            float k = 1.0f - vel->drag * dt;
            if (k < 0.0f) k = 0.0f;
            vel->linear.x *= k;
            vel->linear.z *= k;
        }

        tf->pos.x += vel->linear.x * dt;
        tf->pos.y += vel->linear.y * dt;
        tf->pos.z += vel->linear.z * dt;

        // resting on the ground kills downward velocity so it doesnt accumulate
        // into the floor. the actual ground test lives in the collision pass;
        // here we just trust the flag it set last frame.
        if (col && col->on_ground && vel->linear.y < 0.0f)
            vel->linear.y = 0.0f;
    }
}

void ecs_sys_lifetime(ecs_world *w, float dt, void *user) {
    (void)user;
    ecs_query q;
    ecs_query_begin(&q, w, ecs_with(ECS_CMP_LIFETIME, -1), 0);

    while (ecs_query_next(&q)) {
        ecs_lifetime *lt = ecs_query_get(&q, ECS_CMP_LIFETIME);
        lt->remaining -= dt;
        if (lt->remaining <= 0.0f)
            ecs_destroy(w, q.e);     // deferred while inside the tick
    }
}

void ecs_sys_health(ecs_world *w, float dt, void *user) {
    (void)user;
    ecs_query q;
    ecs_query_begin(&q, w, ecs_with(ECS_CMP_HEALTH, -1), 0);

    while (ecs_query_next(&q)) {
        ecs_health *h = ecs_query_get(&q, ECS_CMP_HEALTH);
        if (h->hurt_timer > 0.0f) {
            h->hurt_timer -= dt;
            if (h->hurt_timer < 0.0f) h->hurt_timer = 0.0f;
        }
        if (h->hp <= 0)
            ecs_destroy(w, q.e);
    }
}
