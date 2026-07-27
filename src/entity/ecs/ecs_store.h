#ifndef ENTITY_ECS_STORE_H
#define ENTITY_ECS_STORE_H

#include "ecs_types.h"

// per-component storage. sparse set: a dense packed array of component data
// kept tight for fast iteration, plus a sparse index from entity-slot -> dense
// position. add/remove are O(1) with the usual swap-with-last trick.
//
// i went back and forth between this and an archetype/table layout. archetypes
// iterate faster but moving entities between tables on every add/remove was a
// pain to get right, and we just dont have enough entities for it to matter.

typedef struct {
    void     *dense;       // packed component blobs, elem_size each
    ecs_entity *dense_ent; // dense[i] belongs to entity dense_ent[i]
    uint32_t *sparse;      // sparse[entity_index] -> dense position (+1, 0=none)
    uint32_t  dense_len;
    uint32_t  dense_cap;
    uint32_t  sparse_cap;  // in entity slots
    size_t    elem_size;
} ecs_store;

void  ecs_store_init(ecs_store *s, size_t elem_size, uint32_t reserve);
void  ecs_store_free(ecs_store *s);

// adds (or overwrites) the component for e and returns a pointer to its slot.
// data may be NULL to leave the slot zeroed for the caller to fill.
void *ecs_store_add(ecs_store *s, ecs_entity e, const void *data);
void *ecs_store_get(const ecs_store *s, ecs_entity e);
int   ecs_store_has(const ecs_store *s, ecs_entity e);
void  ecs_store_remove(ecs_store *s, ecs_entity e);

uint32_t   ecs_store_count(const ecs_store *s);
void      *ecs_store_at(const ecs_store *s, uint32_t dense_i);
ecs_entity ecs_store_entity_at(const ecs_store *s, uint32_t dense_i);

#endif
