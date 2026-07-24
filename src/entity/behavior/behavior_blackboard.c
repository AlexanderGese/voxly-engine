#include "behavior_blackboard.h"

#include <stdlib.h>
#include <string.h>

// the hashmap can't grow our boxed values around for us so we keep our own
// little arena-ish array and hand out indices. when a key is overwritten we
// reuse its slot. removal tombstones the slot in the map but we don't bother
// reclaiming pool slots; trees are short lived enough that it doesn't matter.

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
}

void behavior_bb_set_float(behavior_blackboard *bb, const char *key, float v) {
    bb_value *s = slot_for(bb, key);
    if (!s) return;
    s->type = BB_FLOAT;
    s->as.f = v;
}

void behavior_bb_set_bool(behavior_blackboard *bb, const char *key, int v) {
    bb_value *s = slot_for(bb, key);
    if (!s) return;
    s->type = BB_BOOL;
    s->as.b = v ? 1 : 0;
}

void behavior_bb_set_vec3(behavior_blackboard *bb, const char *key, vec3 v) {
    bb_value *s = slot_for(bb, key);
    if (!s) return;
    s->type = BB_VEC3;
    s->as.v = v;
}

void behavior_bb_set_ptr(behavior_blackboard *bb, const char *key, void *v) {
    bb_value *s = slot_for(bb, key);
    if (!s) return;
    s->type = BB_PTR;
    s->as.p = v;
}

void behavior_bb_set_entity(behavior_blackboard *bb, const char *key, uint32_t id) {
    bb_value *s = slot_for(bb, key);
    if (!s) return;
    s->type = BB_ENTITY;
    s->as.eid = id;
}

int behavior_bb_get_int(const behavior_blackboard *bb, const char *key, int fb) {
    const bb_value *s = slot_get(bb, key);
    return (s && s->type == BB_INT) ? s->as.i : fb;
}

float behavior_bb_get_float(const behavior_blackboard *bb, const char *key, float fb) {
    const bb_value *s = slot_get(bb, key);
    return (s && s->type == BB_FLOAT) ? s->as.f : fb;
}

int behavior_bb_get_bool(const behavior_blackboard *bb, const char *key, int fb) {
    const bb_value *s = slot_get(bb, key);
    return (s && s->type == BB_BOOL) ? s->as.b : fb;
}

vec3 behavior_bb_get_vec3(const behavior_blackboard *bb, const char *key, vec3 fb) {
    const bb_value *s = slot_get(bb, key);
    return (s && s->type == BB_VEC3) ? s->as.v : fb;
}

void *behavior_bb_get_ptr(const behavior_blackboard *bb, const char *key) {
    const bb_value *s = slot_get(bb, key);
    return (s && s->type == BB_PTR) ? s->as.p : NULL;
}

uint32_t behavior_bb_get_entity(const behavior_blackboard *bb, const char *key) {
    const bb_value *s = slot_get(bb, key);
    return (s && s->type == BB_ENTITY) ? s->as.eid : 0u;
}

int behavior_bb_has(const behavior_blackboard *bb, const char *key) {
    return slot_get(bb, key) != NULL;
}

void behavior_bb_remove(behavior_blackboard *bb, const char *key) {
    // just drop the lookup. the boxed value lingers in the pool but is
    // unreachable, which is fine until the next clear.
    hashmap_del(&bb->map, behavior_bb_hash(key));
}
