#ifndef ENTITY_ECS_TYPES_H
#define ENTITY_ECS_TYPES_H

#include <stdint.h>
#include <stddef.h>

// shared types for the little ecs. the old entity.h "one fat struct tagged by
// type" thing didnt scale past a few mob kinds so this is the slow migration
// to component arrays. dont rip out entity.h yet, half the game still uses it.

// entity handle. 20 bits index, 12 bits generation, packed in a u32.
// generation lets us detect a stale handle after the slot got recycled --
// learned that one the hard way when a despawned mob kept getting ticked.
typedef uint32_t ecs_entity;

#define ECS_NULL            ((ecs_entity)0)
#define ECS_INDEX_BITS      20
#define ECS_GEN_BITS        12
#define ECS_INDEX_MASK      ((1u << ECS_INDEX_BITS) - 1u)
#define ECS_GEN_MASK        ((1u << ECS_GEN_BITS) - 1u)
#define ECS_MAX_ENTITIES    (1u << ECS_INDEX_BITS)

static inline uint32_t ecs_entity_index(ecs_entity e) {
    return e & ECS_INDEX_MASK;
}
static inline uint32_t ecs_entity_gen(ecs_entity e) {
    return (e >> ECS_INDEX_BITS) & ECS_GEN_MASK;
}
static inline ecs_entity ecs_entity_make(uint32_t index, uint32_t gen) {
    return (index & ECS_INDEX_MASK) | ((gen & ECS_GEN_MASK) << ECS_INDEX_BITS);
}

// component ids are just small ints. we cap at 64 so a signature fits in a
// single u64 bitmask -- more than enough, we have like 9 components.
#define ECS_MAX_COMPONENTS  64

typedef uint8_t  ecs_component_id;
typedef uint64_t ecs_signature;    // bit i set => has component i

static inline ecs_signature ecs_sig_set(ecs_signature s, ecs_component_id c) {
    return s | ((uint64_t)1 << c);
}
static inline ecs_signature ecs_sig_clear(ecs_signature s, ecs_component_id c) {
    return s & ~((uint64_t)1 << c);
}
static inline int ecs_sig_has(ecs_signature s, ecs_component_id c) {
    return (s >> c) & 1;
}
// does `s` contain every bit required by `mask`? used by queries.
static inline int ecs_sig_match(ecs_signature s, ecs_signature mask) {
    return (s & mask) == mask;
}

#endif
