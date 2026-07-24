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
typedef struct {
    hashmap map;        // key hash -> bb_entry*
    bb_value *pool;     // boxed values, indexed by slot. plain malloc array.
    size_t    pool_len;
    size_t    pool_cap;
} behavior_blackboard;
void behavior_bb_init(behavior_blackboard *bb);
void behavior_bb_free(behavior_blackboard *bb);
void behavior_bb_clear(behavior_blackboard *bb);
void behavior_bb_set_int   (behavior_blackboard *bb, const char *key, int v);
void behavior_bb_set_float (behavior_blackboard *bb, const char *key, float v);
void behavior_bb_set_bool  (behavior_blackboard *bb, const char *key, int v);
#endif
