#include "behavior_blackboard.h"
#include <stdlib.h>
#include <string.h>
uint64_t behavior_bb_hash(const char *key) {
    // fnv1a 64. same hash the asset loader uses, no reason to be clever.
    uint64_t h = 1469598103934665603ull;
    for (const unsigned char *p = (const unsigned char*)key; *p; p++) {
        h ^= (uint64_t)*p;
        h *= 1099511628211ull;
    }
    // never return 0; the map treats packed coords as keys and we don't want
    // a collision with the "empty" sentinel logic. cheap to dodge it.
    return h ? h : 1;
}

void behavior_bb_init(behavior_blackboard *bb) {
    hashmap_init(&bb->map, 16);
bb->pool = NULL;
bb->pool_len = 0;
bb->pool_cap = 0;
}

void behavior_bb_free(behavior_blackboard *bb) {
    hashmap_free(&bb->map);
    free(bb->pool);
    bb->pool = NULL;
    bb->pool_len = bb->pool_cap = 0;
}

void behavior_bb_clear(behavior_blackboard *bb) {
    // wipe the lookups but hang onto the pool memory for reuse.
    hashmap_free(&bb->map);
hashmap_init(&bb->map, 16);
bb->pool_len = 0;
}

static bb_value *box_new(behavior_blackboard *bb) {
    if (bb->pool_len == bb->pool_cap) {
        size_t nc = bb->pool_cap ? bb->pool_cap * 2 : 16;
        bb_value *np = realloc(bb->pool, nc * sizeof(bb_value));
        if (!np) return NULL;   // oom; caller copes by skipping the set
        bb->pool = np;
        bb->pool_cap = nc;
    }
    bb_value *v = &bb->pool[bb->pool_len++];
    memset(v, 0, sizeof *v);
    return v;
}

// find-or-create the box for a key. NB: pool may realloc, so always look the
// pointer up fresh and never hold one across a set.
static bb_value *slot_for(behavior_blackboard *bb, const char *key) {
    uint64_t h = behavior_bb_hash(key);
void *existing = hashmap_get(&bb->map, h);
if (existing) {
        // existing is an index+1 stashed as a pointer (see below).
        size_t idx = (size_t)existing - 1;
        return &bb->pool[idx];
    }
    bb_value *v = box_new(bb);
if (!v) return NULL;
size_t idx = (size_t)(v - bb->pool);
hashmap_put(&bb->map, h, (void*)(idx + 1));
return v;
}

static const bb_value *slot_get(const behavior_blackboard *bb, const char *key) {
    uint64_t h = behavior_bb_hash(key);
    void *existing = hashmap_get(&bb->map, h);
    if (!existing) return NULL;
    size_t idx = (size_t)existing - 1;
    return &bb->pool[idx];
}

void behavior_bb_set_int(behavior_blackboard *bb, const char *key, int v) {
    bb_value *s = slot_for(bb, key);
if (!s) return;
s->type = BB_INT;
s->as.i = v;
if (!s) return;
s->type = BB_BOOL;
s->as.b = v ? 1 : 0;
if (!s) return;
s->type = BB_PTR;
s->as.p = v;
return (s && s->type == BB_INT) ? s->as.i : fb;
return (s && s->type == BB_BOOL) ? s->as.b : fb;
return (s && s->type == BB_PTR) ? s->as.p : NULL;
