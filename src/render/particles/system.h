#ifndef RENDER_PARTICLES_SYSTEM_H
#define RENDER_PARTICLES_SYSTEM_H

// the front door. a particles_system owns a pool, a set of emitters, the cpu
// vertex scratch, and one gpu buffer. you register emitters, call update()
// once a frame, then render(). everything underneath (pool, sim, sort,
// billboard, gpu) is glued together here.
//
// systems are meant to be coarse: one for "world fx" (block breaks, smoke,
// torches), maybe a separate one for additive glows so we can sort the alpha
// one and skip sorting the additive one. don't make a system per torch.

#include "particle_pool.h"
#include "emitter.h"
#include "simulate.h"
#include "billboard.h"
#include "gpu_buffer.h"
#include "../../math/mat4.h"
#include "../../math/aabb.h"

#define PARTICLES_MAX_EMITTERS 64

typedef struct {
    particles_pool   pool;

    particles_emitter emitters[PARTICLES_MAX_EMITTERS];
    int    emitter_count;

    particles_sim_env env;

    // cpu vertex scratch, sized to pool capacity * 6.
    particles_vertex *scratch;
    int    scratch_cap;
    int    scratch_used;

    particles_gpu_buffer gpu;

    // whether this system needs depth sorting. additive systems set this 0.
    int    needs_sort;

    // cached bounds from the last update, for whole-system frustum culling.
    aabb   bounds;
    int    has_bounds;

    // stats, because you always end up wanting them.
    int    last_spawned;
    int    last_live;
} particles_system;

// capacity is the pool size (clamped to PARTICLES_MAX_POOL). needs_sort=1 for
// alpha-blended fx, 0 for purely additive.
void particles_system_init(particles_system *s, int capacity, int needs_sort);
void particles_system_destroy(particles_system *s);

// register an emitter. returns its index/id, or -1 if full. the system copies
// the emitter in; tweak it afterwards via particles_system_emitter().
int  particles_system_add_emitter(particles_system *s, const particles_emitter *e);
particles_emitter *particles_system_emitter(particles_system *s, int id);

// convenience: fire a one-shot burst from a registered emitter.
int  particles_system_burst(particles_system *s, int emitter_id, int count);

// step: emit, simulate, recompute bounds. does no gpu work.
void particles_system_update(particles_system *s, float dt);

// render: collect -> sort -> billboard -> upload -> draw. the caller binds
// the shader and sets view/proj uniforms; we set blend state and draw. cam_*
// are the camera basis vectors (see render/camera.h) and eye is its position.
void particles_system_render(particles_system *s,
                             vec3 cam_right, vec3 cam_up,
                             vec3 cam_fwd, vec3 eye);

#endif
