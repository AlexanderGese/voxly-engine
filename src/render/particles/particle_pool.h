#ifndef RENDER_PARTICLES_PARTICLE_POOL_H
#define RENDER_PARTICLES_PARTICLE_POOL_H
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
#endif
