#ifndef ENTITY_ECS_COMPONENTS_H
#define ENTITY_ECS_COMPONENTS_H

#include <stdint.h>

#include "ecs_types.h"
#include "../../math/vec3.h"
#include "../../math/aabb.h"

// the actual components. these are the stable set the engine ticks every
// frame. component ids are a fixed enum -- registering them dynamically was
// overkill for a fixed game, so they get hardcoded slots and the registry just
// owns the storage. keep this enum < ECS_MAX_COMPONENTS.

typedef enum {
    ECS_CMP_TRANSFORM = 0,   // position + orientation in the world
    ECS_CMP_VELOCITY,        // linear velocity, integrated by movement system
    ECS_CMP_COLLIDER,        // aabb half-extents + ground flag
    ECS_CMP_HEALTH,          // hp / max + hurt cooldown
    ECS_CMP_AI,              // brain state machine bookkeeping
    ECS_CMP_LIFETIME,        // self-despawn timer (dropped items, projectiles)
    ECS_CMP_RENDERABLE,      // model id + tint, read by entity_render
    ECS_CMP_TAG,             // cheap kind tag, mirrors entity_type
    ECS_CMP_COUNT
} ecs_cmp;

typedef struct {
    vec3  pos;
    float yaw;
    float pitch;
} ecs_transform;

typedef struct {
    vec3  linear;
    float drag;       // per-second velocity damping, 0 = none
} ecs_velocity;

typedef struct {
    vec3 half;        // half extents, box centered on transform.pos + up*half.y
    int  on_ground;
} ecs_collider;

typedef struct {
    int   hp;
    int   max_hp;
    float hurt_timer; // i-frames countdown, seconds
} ecs_health;

typedef struct {
    int   state;      // matches the ai_state enum over in ai.h
    int   target;     // ecs_entity of current target, or ECS_NULL
    float timer;      // generic per-state countdown
    float repath;     // time until pathfinder gets poked again
} ecs_ai;

typedef struct {
    float remaining;  // seconds until auto-despawn
} ecs_lifetime;

typedef struct {
    uint16_t model_id;
    uint8_t  tint_r, tint_g, tint_b;
    uint8_t  flags;   // bit0 = visible, bit1 = casts shadow
} ecs_renderable;

typedef struct {
    uint16_t kind;    // entity_type value, kept loose to avoid the include cycle
} ecs_tag;

// fixed component element sizes, indexed by ecs_cmp. used at world init so the
// registry can size each sparse-set without templates (this is C, after all).
size_t ecs_component_size(ecs_cmp c);
const char *ecs_component_name(ecs_cmp c);

#endif
