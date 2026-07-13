#ifndef RENDER_PARTICLES_PARTICLE_POOL_H
#define RENDER_PARTICLES_PARTICLE_POOL_H

// fixed-capacity particle pool with an intrusive free-list. no per-particle
// malloc — we allocate the whole slab once and hand out slots. dead slots
// chain through a free index stack so alloc/free are O(1).
//
// the pool does NOT compact. live particles can be scattered across the
// slab; the sort + draw paths walk a dense index list instead. tried a
// swap-with-last compaction first but it broke stable per-particle state
// (rotation, seed) so i went with the free-list.

#include "particle_types.h"

typedef struct {
    particles_particle *slots;  // capacity entries, one slab
    int      capacity;
    int      alive;             // number of live particles right now

    int     *free_stack;        // indices of free slots
    int      free_top;          // top of the stack (== count of free slots)

    // dense list of live indices, rebuilt each frame in pool_collect().
    // the sort and draw both consume this so they never touch dead slots.
    int     *live_idx;
    int      live_count;
} particles_pool;

void particles_pool_init(particles_pool *p, int capacity);
void particles_pool_free(particles_pool *p);
void particles_pool_clear(particles_pool *p);

// grab a free slot. returns NULL when full (caller just drops the spawn).
particles_particle *particles_pool_alloc(particles_pool *p);

// hand a slot back. safe to call on an already-dead slot (no-op-ish).
void particles_pool_release(particles_pool *p, int index);

// rebuild live_idx from the alive flags. call once per frame after the sim
// has killed off expired particles. returns the live count.
int  particles_pool_collect(particles_pool *p);

// how full are we, 0..1. handy for emitters that throttle when crowded.
float particles_pool_load(const particles_pool *p);

#endif
