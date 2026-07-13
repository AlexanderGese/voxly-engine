#ifndef RENDER_PARTICLES_SIMULATE_H
#define RENDER_PARTICLES_SIMULATE_H

// the per-frame integrator. walks the pool, ages every live particle,
// integrates velocity under gravity + drag, resolves the over-lifetime
// curves into the particle's color/size, and frees anything that expired.
//
// the sim needs the emitters too because the curves live on the emitter, not
// the particle (see curves.h for why). it looks them up by particle->emitter.

#include "particle_pool.h"
#include "emitter.h"
#include "affector.h"
#include "../../math/aabb.h"

// world hooks the sim can optionally use. set ground_y to enable a flat
// collision plane (cheap stand-in for real voxel collision); leave the
// callback NULL to skip per-voxel checks entirely.
typedef struct {
    int   use_ground;     // clamp particles to ground_y
    float ground_y;
    float bounce;         // 0 = stick/kill, 1 = perfect bounce
    float restitution_loss;

    // optional: returns nonzero if the world cell at (x,y,z) is solid.
    // when set, particles that enter a solid cell get reflected/killed.
    int (*is_solid)(void *user, int x, int y, int z);
    void *user;

    // optional global force fields (wind, vortex...). NULL = none.
    particles_affector_set *affectors;
} particles_sim_env;

void particles_sim_env_default(particles_sim_env *env);

// step every live particle by dt. emitters[] is indexed by emitter id; pass
// emitter_count so we can bounds-check the lookup. dead particles are
// released back into the pool here.
void particles_simulate(particles_pool *pool,
                        const particles_emitter *emitters, int emitter_count,
                        const particles_sim_env *env, float dt);

// recompute the world-space bounds of all live particles. handy for frustum
// culling a whole system at once. returns 0 if there are no live particles.
int  particles_compute_bounds(const particles_pool *pool, aabb *out);

#endif
