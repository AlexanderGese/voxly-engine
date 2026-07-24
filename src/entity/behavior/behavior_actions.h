#ifndef ENTITY_BEHAVIOR_ACTIONS_H
#define ENTITY_BEHAVIOR_ACTIONS_H

#include "behavior_node.h"

// concrete leaves wired to the mob steering helpers in mobs/mob_util.h.
// the ctx->agent is expected to be a voxl_mob*. these read tuning values out
// of the blackboard so one tree can drive different mob kinds.
//
// blackboard keys these read (all optional, sensible defaults baked in):
// "target"        vec3   the point we're chasing/fleeing
// "target_id"     entity who we're angry at, 0 = nobody
// "speed"         float  walk speed m/s
// "sight"         float  detection radius
// "reach"         float  attack range
// "lose"          float  give-up distance
// "wander_radius" float  how far wander picks points
//
// conditions return success/failure; actions may return running.

// --- conditions ---
behavior_status behavior_act_has_target   (behavior_ctx *ctx, void *user);
behavior_status behavior_act_target_in_sight(behavior_ctx *ctx, void *user);
behavior_status behavior_act_target_in_reach(behavior_ctx *ctx, void *user);
behavior_status behavior_act_is_hurt      (behavior_ctx *ctx, void *user);
behavior_status behavior_act_low_health   (behavior_ctx *ctx, void *user);

// --- actions ---
behavior_status behavior_act_chase   (behavior_ctx *ctx, void *user);
behavior_status behavior_act_flee    (behavior_ctx *ctx, void *user);
behavior_status behavior_act_wander  (behavior_ctx *ctx, void *user);
behavior_status behavior_act_face_target(behavior_ctx *ctx, void *user);
behavior_status behavior_act_attack  (behavior_ctx *ctx, void *user);
behavior_status behavior_act_idle    (behavior_ctx *ctx, void *user);

#endif
