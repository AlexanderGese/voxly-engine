#include "behavior_actions.h"
#include "behavior_tree.h"
#include "../mobs/mob_common.h"
#include "../mobs/mob_util.h"

#include <math.h>

// pull the agent as a mob. every leaf bails to failure if there's no agent so
// a tree wired to the wrong context object degrades instead of segfaulting.
static voxl_mob *mob_of(behavior_ctx *ctx) {
    return (voxl_mob *)ctx->agent;
}

static vec3 bb_target(behavior_ctx *ctx) {
    return behavior_bb_get_vec3(ctx->bb, "target", VEC3_ZERO);
}

// ---------------------------------------------------------------- conditions

behavior_status behavior_act_has_target(behavior_ctx *ctx, void *user) {
    (void)user;
    // a target counts if we have an entity id or the bb holds a target point.
    if (behavior_bb_get_entity(ctx->bb, "target_id") != 0) return BEHAVIOR_SUCCESS;
    if (behavior_bb_has(ctx->bb, "target"))                 return BEHAVIOR_SUCCESS;
    return BEHAVIOR_FAILURE;
}

behavior_status behavior_act_target_in_sight(behavior_ctx *ctx, void *user) {
    (void)user;
    voxl_mob *m = mob_of(ctx);
    if (!m) return BEHAVIOR_FAILURE;
    float sight = behavior_bb_get_float(ctx->bb, "sight", 16.0f);
    float d = voxl_mob_flat_dist(m->pos, bb_target(ctx));
    return d <= sight ? BEHAVIOR_SUCCESS : BEHAVIOR_FAILURE;
}

behavior_status behavior_act_target_in_reach(behavior_ctx *ctx, void *user) {
    (void)user;
    voxl_mob *m = mob_of(ctx);
    if (!m) return BEHAVIOR_FAILURE;
    float reach = behavior_bb_get_float(ctx->bb, "reach", 1.6f);
    float d = voxl_mob_flat_dist(m->pos, bb_target(ctx));
    return d <= reach ? BEHAVIOR_SUCCESS : BEHAVIOR_FAILURE;
}

behavior_status behavior_act_is_hurt(behavior_ctx *ctx, void *user) {
    (void)user;
    voxl_mob *m = mob_of(ctx);
    if (!m) return BEHAVIOR_FAILURE;
    return m->hurt_timer > 0.0f ? BEHAVIOR_SUCCESS : BEHAVIOR_FAILURE;
}

behavior_status behavior_act_low_health(behavior_ctx *ctx, void *user) {
    (void)user;
    voxl_mob *m = mob_of(ctx);
    if (!m || m->max_health <= 0) return BEHAVIOR_FAILURE;
    // "low" defaults to a third of max; overridable as a fraction.
    float frac = behavior_bb_get_float(ctx->bb, "low_frac", 0.34f);
    float ratio = (float)m->health / (float)m->max_health;
    return ratio <= frac ? BEHAVIOR_SUCCESS : BEHAVIOR_FAILURE;
}

// ------------------------------------------------------------------- actions

behavior_status behavior_act_chase(behavior_ctx *ctx, void *user) {
    (void)user;
    voxl_mob *m = mob_of(ctx);
    if (!m) return BEHAVIOR_FAILURE;

    vec3 tgt = bb_target(ctx);
    float speed = behavior_bb_get_float(ctx->bb, "speed", 2.4f);
    float reach = behavior_bb_get_float(ctx->bb, "reach", 1.6f);
    float lose  = behavior_bb_get_float(ctx->bb, "lose", 22.0f);

    float d = voxl_mob_flat_dist(m->pos, tgt);
    if (d > lose) return BEHAVIOR_FAILURE;      // lost them, let selector move on
    if (d <= reach) return BEHAVIOR_SUCCESS;    // arrived, hand off to attack

    // steer: turn toward the target then step forward. mirrors the wander
    // steering so movement feels consistent across behaviors.
    float want = voxl_mob_yaw_to(m->pos, tgt);
    m->yaw = voxl_mob_turn_toward(m->yaw, want, 6.0f * ctx->dt);
    voxl_mob_walk_forward(m, speed, ctx->dt);
    m->state = VOXL_MS_CHASE;
    return BEHAVIOR_RUNNING;
}

behavior_status behavior_act_flee(behavior_ctx *ctx, void *user) {
    (void)user;
    voxl_mob *m = mob_of(ctx);
    if (!m) return BEHAVIOR_FAILURE;

    vec3 tgt = bb_target(ctx);
    float speed = behavior_bb_get_float(ctx->bb, "speed", 3.0f);
    float safe  = behavior_bb_get_float(ctx->bb, "safe", 14.0f);

    float d = voxl_mob_flat_dist(m->pos, tgt);
    if (d >= safe) return BEHAVIOR_SUCCESS;     // far enough, calm down

    // run directly away: face opposite of the target then walk.
    float toward = voxl_mob_yaw_to(m->pos, tgt);
    float away = voxl_mob_wrap_angle(toward + 3.14159265358979f);
    m->yaw = voxl_mob_turn_toward(m->yaw, away, 8.0f * ctx->dt);
    voxl_mob_walk_forward(m, speed, ctx->dt);
    m->state = VOXL_MS_FLEE;
    return BEHAVIOR_RUNNING;
}

behavior_status behavior_act_wander(behavior_ctx *ctx, void *user) {
    (void)user;
    voxl_mob *m = mob_of(ctx);
    if (!m) return BEHAVIOR_FAILURE;

    float speed = behavior_bb_get_float(ctx->bb, "speed", 1.4f);
    float radius = behavior_bb_get_float(ctx->bb, "wander_radius", 8.0f);

    // pick a fresh point when we don't have one or the dwell timer expired.
    if (!m->has_wander_target || m->state_timer <= 0.0f) {
        // cheap deterministic jitter from position + tick count; good enough
        // for ambient wandering without dragging the rng module in here.
        unsigned h = (unsigned)(m->id * 2654435761u) ^ (unsigned)ctx->tree->tick_count;
        float ang = ((float)(h & 0xffff) / 65535.0f) * 6.2831853f - 3.14159265f;
        float dist = 1.0f + ((float)((h >> 16) & 0xffff) / 65535.0f) * radius;
        m->wander_target.x = m->pos.x + sinf(ang) * dist;
        m->wander_target.y = m->pos.y;
        m->wander_target.z = m->pos.z - cosf(ang) * dist;
        m->has_wander_target = 1;
        m->state_timer = 2.0f + ((float)(h & 0xff) / 255.0f) * 3.0f;
        m->state = VOXL_MS_WANDER;
        return BEHAVIOR_RUNNING;
    }

    m->state_timer -= ctx->dt;
    float d = voxl_mob_flat_dist(m->pos, m->wander_target);
    if (d < 0.6f) {
        m->has_wander_target = 0;
        return BEHAVIOR_SUCCESS;                // reached the spot
    }

    float want = voxl_mob_yaw_to(m->pos, m->wander_target);
    m->yaw = voxl_mob_turn_toward(m->yaw, want, 4.0f * ctx->dt);
    voxl_mob_walk_forward(m, speed, ctx->dt);
    return BEHAVIOR_RUNNING;
}

behavior_status behavior_act_face_target(behavior_ctx *ctx, void *user) {
    (void)user;
    voxl_mob *m = mob_of(ctx);
    if (!m) return BEHAVIOR_FAILURE;
    float want = voxl_mob_yaw_to(m->pos, bb_target(ctx));
    m->yaw = voxl_mob_turn_toward(m->yaw, want, 10.0f * ctx->dt);
    // succeed once we're roughly pointed at it (within ~0.1 rad).
    float err = fabsf(voxl_mob_wrap_angle(want - m->yaw));
    return err < 0.1f ? BEHAVIOR_SUCCESS : BEHAVIOR_RUNNING;
}

behavior_status behavior_act_attack(behavior_ctx *ctx, void *user) {
    (void)user;
    voxl_mob *m = mob_of(ctx);
    if (!m) return BEHAVIOR_FAILURE;

    float reach = behavior_bb_get_float(ctx->bb, "reach", 1.6f);
    float d = voxl_mob_flat_dist(m->pos, bb_target(ctx));
    if (d > reach) return BEHAVIOR_FAILURE;     // they stepped out, back to chase

    m->state = VOXL_MS_ATTACK;
    if (m->attack_cd > 0.0f) {
        m->attack_cd -= ctx->dt;
        return BEHAVIOR_RUNNING;                // mid swing / between swings
    }
    // land a hit: stamp the cooldown and report damage intent on the bb so the
    // game glue can apply it. we don't reach into combat from here.
    float cd = behavior_bb_get_float(ctx->bb, "attack_cd", 1.0f);
    m->attack_cd = cd;
    behavior_bb_set_bool(ctx->bb, "did_attack", 1);
    return BEHAVIOR_SUCCESS;
}

behavior_status behavior_act_idle(behavior_ctx *ctx, void *user) {
    (void)user;
    voxl_mob *m = mob_of(ctx);
    if (!m) return BEHAVIOR_FAILURE;
    // bleed off horizontal velocity so an idling mob actually stops.
    m->vel.x *= 0.8f;
    m->vel.z *= 0.8f;
    m->state = VOXL_MS_IDLE;
    return BEHAVIOR_RUNNING;
}
