#include "ecs_prefab.h"
#include "ecs_components.h"

#include "../ai.h"   // for the AI_* state enum

ecs_entity ecs_prefab_mob(ecs_world *w, uint16_t kind, vec3 pos,
                          float width, float height, int max_hp) {
    ecs_entity e = ecs_create(w);
    if (e == ECS_NULL) return ECS_NULL;

    ecs_transform tf = { .pos = pos, .yaw = 0.0f, .pitch = 0.0f };
    ecs_add(w, e, ECS_CMP_TRANSFORM, &tf);

    ecs_velocity vel = { .linear = VEC3_ZERO, .drag = 6.0f };
    ecs_add(w, e, ECS_CMP_VELOCITY, &vel);

    // collider box centered horizontally, sitting on the feet. half.y is half
    // the height so the aabb spans [pos.y, pos.y + height].
    ecs_collider col = {
        .half = { width * 0.5f, height * 0.5f, width * 0.5f },
        .on_ground = 0,
    };
    ecs_add(w, e, ECS_CMP_COLLIDER, &col);

    ecs_health hp = { .hp = max_hp, .max_hp = max_hp, .hurt_timer = 0.0f };
    ecs_add(w, e, ECS_CMP_HEALTH, &hp);

    ecs_ai ai = { .state = AI_IDLE, .target = (int)ECS_NULL,
                  .timer = 0.0f, .repath = 0.0f };
    ecs_add(w, e, ECS_CMP_AI, &ai);

    ecs_tag tag = { .kind = kind };
    ecs_add(w, e, ECS_CMP_TAG, &tag);

    ecs_renderable rnd = {
        .model_id = kind,           // model atlas is keyed by kind for now
        .tint_r = 255, .tint_g = 255, .tint_b = 255,
        .flags  = 0x3,              // visible | casts shadow
    };
    ecs_add(w, e, ECS_CMP_RENDERABLE, &rnd);
    return e;
}

ecs_entity ecs_prefab_item_drop(ecs_world *w, uint16_t item_id, vec3 pos,
                                float lifetime_s) {
    ecs_entity e = ecs_create(w);
    if (e == ECS_NULL) return ECS_NULL;

    ecs_transform tf = { .pos = pos, .yaw = 0.0f, .pitch = 0.0f };
    ecs_add(w, e, ECS_CMP_TRANSFORM, &tf);

    // a tiny upward pop so freshly dropped items hop out of the block instead
    // of clipping into the floor. classic.
    ecs_velocity vel = { .linear = { 0.0f, 2.0f, 0.0f }, .drag = 1.5f };
    ecs_add(w, e, ECS_CMP_VELOCITY, &vel);

    ecs_collider col = { .half = { 0.125f, 0.125f, 0.125f }, .on_ground = 0 };
    ecs_add(w, e, ECS_CMP_COLLIDER, &col);

    ecs_lifetime lt = { .remaining = lifetime_s };
    ecs_add(w, e, ECS_CMP_LIFETIME, &lt);

    ecs_renderable rnd = {
        .model_id = item_id,
        .tint_r = 255, .tint_g = 255, .tint_b = 255,
        .flags  = 0x1,              // visible, no shadow on tiny drops
    };
    ecs_add(w, e, ECS_CMP_RENDERABLE, &rnd);
    return e;
}

ecs_entity ecs_prefab_marker(ecs_world *w, vec3 pos, uint16_t model_id) {
    ecs_entity e = ecs_create(w);
    if (e == ECS_NULL) return ECS_NULL;

    ecs_transform tf = { .pos = pos, .yaw = 0.0f, .pitch = 0.0f };
    ecs_add(w, e, ECS_CMP_TRANSFORM, &tf);

    ecs_renderable rnd = {
        .model_id = model_id,
        .tint_r = 255, .tint_g = 255, .tint_b = 255,
        .flags  = 0x1,
    };
    ecs_add(w, e, ECS_CMP_RENDERABLE, &rnd);
    return e;
}
