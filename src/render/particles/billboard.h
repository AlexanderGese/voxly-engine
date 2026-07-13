#ifndef RENDER_PARTICLES_BILLBOARD_H
#define RENDER_PARTICLES_BILLBOARD_H
#include "particle_pool.h"
#include "emitter.h"
#include "../../math/vec3.h"
typedef struct {
    vec3 right;
    vec3 up;
    vec3 forward;   // camera forward, for the velocity-aligned case
    vec3 eye;
} particles_billboard_cam;
void particles_billboard_cam_make(particles_billboard_cam *bc,
                                  vec3 cam_right, vec3 cam_up,
                                  vec3 cam_fwd, vec3 eye);
#endif
