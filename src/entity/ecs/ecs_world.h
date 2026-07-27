#ifndef ENTITY_ECS_WORLD_H
#define ENTITY_ECS_WORLD_H

#include "ecs_types.h"
#include "ecs_entity_pool.h"
#include "ecs_store.h"
#include "ecs_components.h"

// the registry. owns the entity id pool and one sparse-set store per component.
// everything else (queries, systems) operates on this. there is normally one
// per game world but nothing stops you spinning up a second for, say, a menu
// preview scene -- which is exactly why it isnt a global.

// deferred destroys: ticking a system while it deletes entities out from under
// the iteration it's in is a recipe for crashes, so destroys requested during a
// tick get queued and flushed at the end. ecs_destroy() is safe mid-iteration.

typedef struct {
    ecs_entity_pool pool;
    ecs_store       stores[ECS_CMP_COUNT];

    ecs_entity     *destroy_queue;   // darray of pending destroys
    int             deferring;       // >0 while inside a system tick
} ecs_world;

void        ecs_world_init(ecs_world *w, uint32_t reserve);
void        ecs_world_free(ecs_world *w);
// wipe every entity + component but keep the allocations. used by snapshot load
// and when leaving a world without tearing the whole registry down.
void        ecs_world_clear(ecs_world *w);

ecs_entity  ecs_create(ecs_world *w);
void        ecs_destroy(ecs_world *w, ecs_entity e);
int         ecs_alive(const ecs_world *w, ecs_entity e);
uint32_t    ecs_count(const ecs_world *w);

// component access. add returns a pointer to the (zeroed or copied) slot.
void       *ecs_add(ecs_world *w, ecs_entity e, ecs_cmp c, const void *data);
void       *ecs_get(ecs_world *w, ecs_entity e, ecs_cmp c);
int         ecs_has(const ecs_world *w, ecs_entity e, ecs_cmp c);
void        ecs_remove(ecs_world *w, ecs_entity e, ecs_cmp c);

// flushes the deferred-destroy queue. systems runner calls this for you.
void        ecs_flush_destroyed(ecs_world *w);

#endif
