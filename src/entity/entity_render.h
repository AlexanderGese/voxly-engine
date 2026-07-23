#ifndef ENTITY_RENDER_H
#define ENTITY_RENDER_H

#include "mob.h"
#include "../render/gl.h"
#include "../render/camera.h"

// draws entities as simple colored boxes (billboards would be better but
// this gets the silhouettes on screen and then we can move on)

typedef struct {
    glid prog;
    glid vao;
    glid vbo;
} entity_renderer;

int  entity_renderer_init(entity_renderer *er);
void entity_renderer_destroy(entity_renderer *er);
void entity_renderer_draw(entity_renderer *er, mob_registry *mr, const camera *cam);

#endif
