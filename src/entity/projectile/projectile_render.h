#ifndef ENTITY_PROJECTILE_RENDER_H
#define ENTITY_PROJECTILE_RENDER_H

#include "projectile_pool.h"
#include "../../render/gl.h"
#include "../../render/camera.h"

// draws projectiles as little oriented sticks (a thin stretched box aimed down
// the heading). same spirit as entity_render: not pretty, but you can see the
// arrows arc and that's what we need before anyone writes a real model loader.
// one shared unit-box vao, a model matrix per projectile, instanced-by-loop.

typedef struct {
    glid prog;
    glid vao;
    glid vbo;
    int  ready;
} projectile_renderer;

int  projectile_renderer_init(projectile_renderer *pr);
void projectile_renderer_destroy(projectile_renderer *pr);

// draw every live (flying or stuck) projectile in the pool. stuck ones render at
// their embedded tip and dont wobble; flying ones spin their fletching a bit.
void projectile_renderer_draw(projectile_renderer *pr,
                              const projectile_pool *pool,
                              const camera *cam);

// build the orientation matrix that aims +z down a heading. exposed so the demo
// and any debug overlay can reuse it. up is world-up unless the shot is vertical.
mat4 projectile_render_orient(vec3 forward, vec3 pos, float spin, vec3 scale);

#endif
