#ifndef RENDER_PARTICLES_SORT_H
#define RENDER_PARTICLES_SORT_H

// depth sort for alpha-blended particles. transparent billboards have to be
// drawn back-to-front or the blending looks wrong (near particles eat the
// ones behind them). additive blending doesn't care about order, so the
// system skips this for additive systems.
//
// we sort the pool's live_idx array in place, ordered by squared distance to
// the camera, far first. uses an lsd radix sort on a quantized depth key —
// for a few thousand particles that's a lot faster than qsort and, more
// importantly, branch-free and stable.

#include "particle_pool.h"
#include "../../math/vec3.h"

// reorder pool->live_idx so the farthest particle comes first. call after
// particles_pool_collect(). cam is the camera/eye position.
void particles_sort_back_to_front(particles_pool *pool, vec3 cam);

// the radix primitive, exposed for testing. sorts `n` (key,value) pairs by
// the 32-bit key ascending, scratch must hold n entries each.
void particles_radix_u32(uint32_t *keys, int *vals,
                         uint32_t *key_scratch, int *val_scratch, int n);

#endif
