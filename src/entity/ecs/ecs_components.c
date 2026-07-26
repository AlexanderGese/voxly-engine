#include "ecs_components.h"

// the sizes/names tables. plain switch instead of a designated-init array so a
// missing case is a compile warning rather than a silent zero. learned that
// after a 0-sized component quietly corrupted every store that shared a slot.

size_t ecs_component_size(ecs_cmp c) {
    switch (c) {
    case ECS_CMP_TRANSFORM:  return sizeof(ecs_transform);
    case ECS_CMP_VELOCITY:   return sizeof(ecs_velocity);
    case ECS_CMP_COLLIDER:   return sizeof(ecs_collider);
    case ECS_CMP_HEALTH:     return sizeof(ecs_health);
    case ECS_CMP_AI:         return sizeof(ecs_ai);
    case ECS_CMP_LIFETIME:   return sizeof(ecs_lifetime);
    case ECS_CMP_RENDERABLE: return sizeof(ecs_renderable);
    case ECS_CMP_TAG:        return sizeof(ecs_tag);
    case ECS_CMP_COUNT:      return 0;
    }
    return 0;
}

const char *ecs_component_name(ecs_cmp c) {
    switch (c) {
    case ECS_CMP_TRANSFORM:  return "transform";
    case ECS_CMP_VELOCITY:   return "velocity";
    case ECS_CMP_COLLIDER:   return "collider";
    case ECS_CMP_HEALTH:     return "health";
    case ECS_CMP_AI:         return "ai";
    case ECS_CMP_LIFETIME:   return "lifetime";
    case ECS_CMP_RENDERABLE: return "renderable";
    case ECS_CMP_TAG:        return "tag";
    case ECS_CMP_COUNT:      return "?";
    }
    return "?";
}
