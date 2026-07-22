#ifndef RENDER_VOLUMETRIC_QUAD_H
#define RENDER_VOLUMETRIC_QUAD_H

#include "../gl.h"

// the obligatory fullscreen triangle. every screen-space pass in here draws it.
// same oversized-tri trick the ssao pass uses; the passthrough vert derives uv
// from clip position so we don't even ship texcoords.

typedef struct {
    glid vao;
    glid vbo;
} volumetric_quad;

int  volumetric_quad_init(volumetric_quad *q);
void volumetric_quad_destroy(volumetric_quad *q);
void volumetric_quad_draw(const volumetric_quad *q);

#endif
