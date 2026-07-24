#include "behavior_actions.h"
#include "behavior_tree.h"
#include "../mobs/mob_common.h"
#include "../mobs/mob_util.h"
#include <math.h>
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
voxl_mob *m = mob_of(ctx);
if (!m) return BEHAVIOR_FAILURE;
return m->hurt_timer > 0.0f ? BEHAVIOR_SUCCESS : BEHAVIOR_FAILURE;
voxl_mob *m = mob_of(ctx);
if (!m) return BEHAVIOR_FAILURE;
vec3 tgt = bb_target(ctx);
float speed = behavior_bb_get_float(ctx->bb, "speed", 2.4f);
float reach = behavior_bb_get_float(ctx->bb, "reach", 1.6f);
float lose  = behavior_bb_get_float(ctx->bb, "lose", 22.0f);
float d = voxl_mob_flat_dist(m->pos, tgt);
if (d > lose) return BEHAVIOR_FAILURE;
if (d <= reach) return BEHAVIOR_SUCCESS;
float want = voxl_mob_yaw_to(m->pos, tgt);
m->yaw = voxl_mob_turn_toward(m->yaw, want, 6.0f * ctx->dt);
voxl_mob_walk_forward(m, speed, ctx->dt);
m->state = VOXL_MS_CHASE;
return BEHAVIOR_RUNNING;
voxl_mob *m = mob_of(ctx);
if (!m) return BEHAVIOR_FAILURE;
float speed = behavior_bb_get_float(ctx->bb, "speed", 1.4f);
float radius = behavior_bb_get_float(ctx->bb, "wander_radius", 8.0f);
float d = voxl_mob_flat_dist(m->pos, m->wander_target);
m->yaw = voxl_mob_turn_toward(m->yaw, want, 4.0f * ctx->dt);
voxl_mob_walk_forward(m, speed, ctx->dt);
return BEHAVIOR_RUNNING;
voxl_mob *m = mob_of(ctx);
if (!m) return BEHAVIOR_FAILURE;
float reach = behavior_bb_get_float(ctx->bb, "reach", 1.6f);
float d = voxl_mob_flat_dist(m->pos, bb_target(ctx));
if (d > reach) return BEHAVIOR_FAILURE;
m->state = VOXL_MS_ATTACK;
m->attack_cd = cd;
behavior_bb_set_bool(ctx->bb, "did_attack", 1);
return BEHAVIOR_SUCCESS;
