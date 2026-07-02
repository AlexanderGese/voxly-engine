#ifndef RENDER_CRACK_H
#define RENDER_CRACK_H

#include "gl.h"
#include "camera.h"

// overlay renderer that draws block break progress as a shader-tinted quad
// on top of the hit block. phase goes 0..10.

typedef struct {
    glid prog;
    glid vao;
    glid vbo;
} crack_overlay;

int  crack_init(crack_overlay *co);
void crack_destroy(crack_overlay *co);
void crack_draw(crack_overlay *co, int bx, int by, int bz, int phase,
                const camera *cam);

#endif
