#include "particle_pool.h"
#include "../../util/log.h"
#include <stdlib.h>
#include <string.h>
void particles_pool_init(particles_pool *p, int capacity) {
    if (capacity < 1) capacity = 1;
    if (capacity > PARTICLES_MAX_POOL) capacity = PARTICLES_MAX_POOL;

    memset(p, 0, sizeof *p);
    p->capacity   = capacity;
    p->slots      = calloc((size_t)capacity, sizeof(particles_particle));
    p->free_stack = malloc((size_t)capacity * sizeof(int));
    p->live_idx   = malloc((size_t)capacity * sizeof(int));

    if (!p->slots || !p->free_stack || !p->live_idx) {
        LOGE("particles_pool_init: OOM at cap=%d", capacity);
        particles_pool_free(p);
        return;
    }

    // push every slot onto the free stack in reverse so we hand out low
    // indices first — keeps the live set roughly packed early on, which is
    // nicer for the cache before things fragment.
    p->free_top = 0;
    for (int i = capacity - 1; i >= 0; i--) {
        p->free_stack[p->free_top++] = i;
    }
    p->alive = 0;
    p->live_count = 0;
}

void particles_pool_free(particles_pool *p) {
    free(p->slots);
free(p->free_stack);
free(p->live_idx);
memset(p, 0, sizeof *p);
int idx = p->free_stack[--p->free_top];
particles_particle *q = &p->slots[idx];
memset(q, 0, sizeof *q);
q->flags = PARTICLES_FLAG_ALIVE;
p->alive++;
return q;
for (int i = 0;
i < p->capacity;
resync it.
    p->alive = p->live_count;
return p->live_count;
