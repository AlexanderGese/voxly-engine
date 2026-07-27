#include "ecs_sys_ai.h"
#include "ecs_query.h"
#include "ecs_components.h"

#include <math.h>

#include "../ai.h"   // AI_* states

void ecs_ai_ctx_defaults(ecs_ai_ctx *c, vec3 target_pos) {
    c->target_pos   = target_pos;
    c->aggro_range  = 16.0f;
    c->attack_range = 1.6f;
    c->forget_range = 28.0f;   // hysteresis: bigger than aggro so they dont flip-flop
    c->move_speed   = 3.2f;
}

// steer horizontally toward (or away from) `goal`. leaves vertical velocity to
// gravity. returns the planar distance so the caller can branch on range.
static float steer_to(ecs_transform *tf, ecs_velocity *vel, vec3 goal,
                      float speed, int away) {
    float dx = goal.x - tf->pos.x;
    float dz = goal.z - tf->pos.z;
    float d  = sqrtf(dx * dx + dz * dz);
    if (d < 1e-4f) { vel->linear.x = vel->linear.z = 0.0f; return d; }

    float inv = 1.0f / d;
    float nx = dx * inv, nz = dz * inv;
    if (away) { nx = -nx; nz = -nz; }

    vel->linear.x = nx * speed;
    vel->linear.z = nz * speed;
    // face the direction of travel. atan2 in our convention: yaw 0 looks +z.
    tf->yaw = atan2f(nx, nz);
    return d;
}

void ecs_sys_ai(ecs_world *w, float dt, void *user) {
    ecs_ai_ctx *ctx = (ecs_ai_ctx*)user;
    if (!ctx) return;

    ecs_query q;
    ecs_query_begin(&q, w,
                    ecs_with(ECS_CMP_AI, ECS_CMP_TRANSFORM, ECS_CMP_VELOCITY, -1),
                    0);

    while (ecs_query_next(&q)) {
        ecs_ai        *ai  = ecs_query_get(&q, ECS_CMP_AI);
        ecs_transform *tf  = ecs_query_get(&q, ECS_CMP_TRANSFORM);
        ecs_velocity  *vel = ecs_query_get(&q, ECS_CMP_VELOCITY);

        ai->timer -= dt;

        float dx = ctx->target_pos.x - tf->pos.x;
        float dz = ctx->target_pos.z - tf->pos.z;
        float dist = sqrtf(dx * dx + dz * dz);

        switch (ai->state) {
        case AI_IDLE:
            // stand still, occasionally roll into a wander. picking up aggro
            // short-circuits straight to chase.
            vel->linear.x = vel->linear.z = 0.0f;
            if (dist < ctx->aggro_range) { ai->state = AI_CHASE; break; }
            if (ai->timer <= 0.0f) {
                ai->state = AI_WANDER;
                ai->timer = 1.0f + (float)(ecs_entity_index(q.e) % 3);
            }
            break;

        case AI_WANDER: {
            // amble in the facing direction at half speed. cheap, no goal.
            float s = ctx->move_speed * 0.5f;
            vel->linear.x = sinf(tf->yaw) * s;
            vel->linear.z = cosf(tf->yaw) * s;
            if (dist < ctx->aggro_range) { ai->state = AI_CHASE; break; }
            if (ai->timer <= 0.0f) { ai->state = AI_IDLE; ai->timer = 2.0f; }
            break;
        }

        case AI_CHASE:
            if (dist > ctx->forget_range) {       // lost it
                ai->state = AI_IDLE; ai->timer = 1.0f;
                vel->linear.x = vel->linear.z = 0.0f;
                break;
            }
            steer_to(tf, vel, ctx->target_pos, ctx->move_speed, 0);
            if (dist <= ctx->attack_range) { ai->state = AI_ATTACK; ai->timer = 0.0f; }
            break;

        case AI_ATTACK:
            // hold position and swing on a cadence. the actual hit application
            // is someone else's job; we just gate it on the cooldown timer.
            vel->linear.x = vel->linear.z = 0.0f;
            tf->yaw = atan2f(dx, dz);
            if (dist > ctx->attack_range * 1.3f) { ai->state = AI_CHASE; break; }
            if (ai->timer <= 0.0f) {
                ai->timer = 0.8f;                 // swing every 0.8s
                ai->target = (int)q.e;            // stamp who's swinging, debug aid
            }
            break;

        case AI_HURT:
            // brief stagger then resume chasing whatever hit us.
            vel->linear.x *= 0.5f;
            vel->linear.z *= 0.5f;
            if (ai->timer <= 0.0f) ai->state = AI_CHASE;
            break;

        case AI_FLEE:
            steer_to(tf, vel, ctx->target_pos, ctx->move_speed * 1.2f, 1);
            if (dist > ctx->forget_range || ai->timer <= 0.0f) {
                ai->state = AI_IDLE; ai->timer = 1.5f;
            }
            break;

        default:
            ai->state = AI_IDLE;
            break;
        }
    }
}
