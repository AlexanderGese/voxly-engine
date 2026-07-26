#ifndef ENTITY_ECS_ENTITY_POOL_H
#define ENTITY_ECS_ENTITY_POOL_H

#include "ecs_types.h"

// entity id allocator. hands out indices, recycles freed ones, and bumps a
// per-slot generation on free so old handles go stale. classic slot-map.

typedef struct {
    uint16_t      *gen;       // generation per slot index
    ecs_signature *sig;       // component signature per live entity
    uint8_t       *alive;     // 1 if slot currently holds a live entity
    uint32_t      *freelist;  // stack of recycled indices
    uint32_t       free_len;
    uint32_t       cap;       // allocated slots
    uint32_t       count;     // live entities
    uint32_t       hiwater;   // highest index ever handed out (+1)
} ecs_entity_pool;

void        ecs_pool_init(ecs_entity_pool *p, uint32_t initial_cap);
void        ecs_pool_free(ecs_entity_pool *p);

// create returns a fresh handle, recycling a slot if one is free.
ecs_entity  ecs_pool_create(ecs_entity_pool *p);
// destroy invalidates the handle. no-op if already dead / stale.
void        ecs_pool_destroy(ecs_entity_pool *p, ecs_entity e);

int         ecs_pool_alive(const ecs_entity_pool *p, ecs_entity e);

ecs_signature ecs_pool_sig(const ecs_entity_pool *p, ecs_entity e);
void          ecs_pool_set_sig(ecs_entity_pool *p, ecs_entity e, ecs_signature s);

// resurrect a specific handle (index+generation) during snapshot load so the
// rebuilt world hands out the same ids it saved. callers go through the entity
// indices in any order; the freelist gets rebuilt afterward by ecs_pool_reseed.
ecs_entity ecs_pool_restore(ecs_entity_pool *p, uint32_t index, uint32_t gen);
// rebuild the freelist + hiwater so post-load create() doesnt collide with a
// restored slot. call once after all ecs_pool_restore calls.
void       ecs_pool_reseed(ecs_entity_pool *p);

#endif
