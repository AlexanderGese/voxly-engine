#ifndef FX_PARTICLES_H
#define FX_PARTICLES_H

#include "../math/vec3.h"
#include "../render/gl.h"
#include "../render/camera.h"

// cpu particle system. used for block break dust.

#define PARTICLE_MAX 1024

typedef struct {
    vec3  pos;
    vec3  vel;
    float age;
    float life;
    float size;
    float r, g, b;
    int   alive;
} particle;

typedef struct {
    particle list[PARTICLE_MAX];
    int      count;
    glid     vao, vbo, prog;
} particle_system;

int  particles_init(particle_system *ps);
void particles_destroy(particle_system *ps);
void particles_spawn_break(particle_system *ps, vec3 block_center);
void particles_update(particle_system *ps, float dt);
void particles_draw(particle_system *ps, const camera *cam);

#endif
