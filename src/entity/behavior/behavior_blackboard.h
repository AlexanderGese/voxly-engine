#ifndef ENTITY_BEHAVIOR_BLACKBOARD_H
#define ENTITY_BEHAVIOR_BLACKBOARD_H

#include "../../math/vec3.h"
#include "../../util/hashmap.h"
#include <stdint.h>

// shared scratch memory for a behavior tree. leaf nodes read/write it,
// that's how a "find target" node hands a position to a "chase" node without
// the two knowing about each other.
//
// keys are short strings, hashed to u64 (fnv1a) and stored in the engine
// hashmap. values are a little tagged union so we don't malloc per entry.
// the hashmap stores void* so we box each entry in the blackboard's own pool.

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

// setters. overwrite in place if the key already exists.
void behavior_bb_set_int   (behavior_blackboard *bb, const char *key, int v);
void behavior_bb_set_float (behavior_blackboard *bb, const char *key, float v);
void behavior_bb_set_bool  (behavior_blackboard *bb, const char *key, int v);
void behavior_bb_set_vec3  (behavior_blackboard *bb, const char *key, vec3 v);
void behavior_bb_set_ptr   (behavior_blackboard *bb, const char *key, void *v);
void behavior_bb_set_entity(behavior_blackboard *bb, const char *key, uint32_t id);

// getters return a fallback if the key is missing or the wrong type.
int      behavior_bb_get_int   (const behavior_blackboard *bb, const char *key, int fallback);
float    behavior_bb_get_float (const behavior_blackboard *bb, const char *key, float fallback);
int      behavior_bb_get_bool  (const behavior_blackboard *bb, const char *key, int fallback);
vec3     behavior_bb_get_vec3  (const behavior_blackboard *bb, const char *key, vec3 fallback);
void    *behavior_bb_get_ptr   (const behavior_blackboard *bb, const char *key);
uint32_t behavior_bb_get_entity(const behavior_blackboard *bb, const char *key);

int  behavior_bb_has(const behavior_blackboard *bb, const char *key);
void behavior_bb_remove(behavior_blackboard *bb, const char *key);

// exposed because a couple of decorators want to key off the same hash.
uint64_t behavior_bb_hash(const char *key);

#endif
