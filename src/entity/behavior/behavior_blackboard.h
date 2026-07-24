#ifndef ENTITY_BEHAVIOR_BLACKBOARD_H
#define ENTITY_BEHAVIOR_BLACKBOARD_H
#include "../../math/vec3.h"
#include "../../util/hashmap.h"
#include <stdint.h>
typedef enum {
    BB_NONE = 0,
    BB_INT,
    BB_FLOAT,
    BB_BOOL,
    BB_VEC3,
    BB_PTR,
    BB_ENTITY,   // u32 entity/mob id. 0 means "nobody".
} bb_type;
typedef struct {
    bb_type type;
    union {
        int      i;
        float    f;
        int      b;
        vec3     v;
        void    *p;
        uint32_t eid;
    } as;
} bb_value;
#endif
