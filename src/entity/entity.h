#ifndef ENTITY_ENTITY_H
#define ENTITY_ENTITY_H

#include "../math/vec3.h"
#include "../math/aabb.h"
#include <stdint.h>

// very simple entity system: one struct for all kinds, tagged by type.
// id 0 is invalid (used as "no entity").

typedef enum {
    ET_NONE = 0,
    ET_ZOMBIE,
    ET_COW,
    ET_PIG,
    ET_SKELETON,
    ET_SPIDER,
    ET_COUNT
} entity_type;

typedef struct {
    uint32_t    id;
    entity_type type;
    vec3        pos;
    vec3        vel;
    float       yaw;
    int         hp;
    int         max_hp;
    int         on_ground;
    float       hurt_timer;
    float       ai_timer;
    int         ai_state;
    int         target_id;
    int         alive;
} entity;

// factories
entity entity_new(entity_type t, vec3 pos);
aabb   entity_aabb(const entity *e);
float  entity_height(entity_type t);
float  entity_width(entity_type t);
const char *entity_name(entity_type t);

#endif
