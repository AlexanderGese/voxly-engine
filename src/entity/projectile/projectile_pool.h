#ifndef ENTITY_PROJECTILE_POOL_H
#define ENTITY_PROJECTILE_POOL_H

#include "projectile_types.h"

// fixed-capacity pool of live projectiles. flat array, free-list-free: we just
// scan for a FREE slot on alloc, which is fine at this cap (a few hundred). same
// shape as world/item.c's item_world so it reads familiar. stored by value, no
// per-projectile allocation, cache-friendly to iterate.

#define PROJECTILE_POOL_CAP 256

typedef struct {
    projectile slots[PROJECTILE_POOL_CAP];
    int        count;       // live (non-FREE) slot count, for stats/early-out
    uint32_t   spawned;     // lifetime spawn counter, debug
} projectile_pool;

void projectile_pool_init(projectile_pool *pool);

// grab a free slot and zero it to a clean FLYING-ready projectile with the given
// id. returns the slot pointer or NULL if the pool is full (oldest-wins culling
// is the caller's job; we just refuse). id 0 is rejected.
projectile *projectile_pool_alloc(projectile_pool *pool, uint32_t id);

// release a slot back to FREE. safe to call on an already-free slot.
void projectile_pool_free(projectile_pool *pool, projectile *p);

// find a live projectile by id, or NULL. linear; ids arent sorted.
projectile *projectile_pool_find(projectile_pool *pool, uint32_t id);

// reap every SPENT slot, returning it to FREE. call once per frame after the
// update pass. returns how many were reaped.
int projectile_pool_reap(projectile_pool *pool);

// if the pool is full, free the oldest FLYING projectile to make room. returns 1
// if it culled one. lets the world keep firing under spam without a hard cap.
int projectile_pool_cull_oldest(projectile_pool *pool);

#endif
