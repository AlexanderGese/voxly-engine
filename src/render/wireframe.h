#ifndef RENDER_WIREFRAME_H
#define RENDER_WIREFRAME_H

#include "gl.h"
#include "camera.h"
#include "../math/aabb.h"

// helper for drawing aabb wireframes (selected block, debug bounds, etc)

typedef struct {
    glid vao, vbo, prog;
} wireframe;

int  wireframe_init(wireframe *wf);
void wireframe_destroy(wireframe *wf);
void wireframe_draw_aabb(wireframe *wf, aabb a, const camera *cam,
                         float r, float g, float b);

#endif
