#ifndef RENDER_PARTICLES_BILLBOARD_H
#define RENDER_PARTICLES_BILLBOARD_H

// turns live particles into camera-facing quads (two tris, 6 verts each) and
// writes them into a vertex scratch buffer. this is the cpu billboard path —
// we expand on the cpu rather than in a geometry shader because the engine
// targets a plain core profile and i didn't want to depend on gs support.
//
// the buffer is the system's; this module just fills it. atlas uvs are
// derived from the emitter's atlas_tile using the engine's 16x16 tile grid.

#include "particle_pool.h"
#include "emitter.h"
#include "../../math/vec3.h"

// the basis we billboard against. for spherical we use camera right/up; for
// cylindrical we lock up to world-up; velocity stretch builds its own basis.
typedef struct {
    vec3 right;
    vec3 up;
    vec3 forward;   // camera forward, for the velocity-aligned case
    vec3 eye;
} particles_billboard_cam;

void particles_billboard_cam_make(particles_billboard_cam *bc,
                                  vec3 cam_right, vec3 cam_up,
                                  vec3 cam_fwd, vec3 eye);

// fill `out` with verts for the live particles in draw order (pool->live_idx).
// writes at most live_count*6 vertices; returns the number actually written.
// out must have room for pool->live_count * PARTICLES_VERTS_PER entries.
int particles_billboard_build(particles_vertex *out, int out_cap,
                              const particles_pool *pool,
                              const particles_emitter *emitters,
                              int emitter_count,
                              const particles_billboard_cam *cam);

#endif
