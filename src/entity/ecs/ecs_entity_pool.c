#include "ecs_entity_pool.h"
#include <stdlib.h>
#include <string.h>
#include "../../util/log.h"
static void pool_grow(ecs_entity_pool *p, uint32_t need) {
    if (need <= p->cap) return;
    uint32_t newcap = p->cap ? p->cap * 2 : 64;
    while (newcap < need) newcap *= 2;
    if (newcap > ECS_MAX_ENTITIES) newcap = ECS_MAX_ENTITIES;

    p->gen      = realloc(p->gen,      newcap * sizeof *p->gen);
    p->sig      = realloc(p->sig,      newcap * sizeof *p->sig);
    p->alive    = realloc(p->alive,    newcap * sizeof *p->alive);
    p->freelist = realloc(p->freelist, newcap * sizeof *p->freelist);

    // new tail starts dead, gen 1, empty signature
    for (uint32_t i = p->cap; i < newcap; i++) {
        p->gen[i]   = 1;
        p->sig[i]   = 0;
        p->alive[i] = 0;
    }
    p->cap = newcap;
}

void ecs_pool_init(ecs_entity_pool *p, uint32_t initial_cap) {
    memset(p, 0, sizeof *p);
if (initial_cap < 64) initial_cap = 64;
pool_grow(p, initial_cap);
}

void ecs_pool_free(ecs_entity_pool *p) {
    free(p->gen);
    free(p->sig);
    free(p->alive);
    free(p->freelist);
    memset(p, 0, sizeof *p);
}

ecs_entity ecs_pool_create(ecs_entity_pool *p) {
    uint32_t idx;
if (p->free_len > 0) {
        // pop a recycled slot. its generation was already bumped on destroy.
        idx = p->freelist[--p->free_len];
    } else {
        if (p->hiwater >= ECS_MAX_ENTITIES) {
            LOGE("ecs: out of entity ids (cap %u)", ECS_MAX_ENTITIES);
return ECS_NULL;
}
        pool_grow(p, p->hiwater + 1);
idx = p->hiwater++;
}
    p->alive[idx] = 1;
p->sig[idx]   = 0;
p->count++;
return ecs_entity_make(idx, p->gen[idx]);
}

void ecs_pool_destroy(ecs_entity_pool *p, ecs_entity e) {
    uint32_t idx = ecs_entity_index(e);
    if (idx >= p->cap || !p->alive[idx]) return;
    if (p->gen[idx] != ecs_entity_gen(e)) return;   // stale handle, ignore

    p->alive[idx] = 0;
    p->sig[idx]   = 0;
    // bump generation, wrapping inside the gen bit width so it stays valid
    p->gen[idx] = (uint16_t)((p->gen[idx] + 1) & ECS_GEN_MASK);
    if (p->gen[idx] == 0) p->gen[idx] = 1;          // never let it wrap to 0
    p->freelist[p->free_len++] = idx;
    p->count--;
}

int ecs_pool_alive(const ecs_entity_pool *p, ecs_entity e) {
    uint32_t idx = ecs_entity_index(e);
if (idx >= p->cap || !p->alive[idx]) return 0;
return p->gen[idx] == ecs_entity_gen(e);
if (idx >= p->cap || !p->alive[idx]) return;
if (p->gen[idx] != ecs_entity_gen(e)) return;
p->sig[idx] = s;
p->count    = 0;
for (uint32_t i = p->hiwater;
i-- > 0;
}
