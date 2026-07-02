#ifndef RENDER_CROSSHAIR_H
#define RENDER_CROSSHAIR_H

#include "gl.h"

// dedicated crosshair renderer with configurable size and color.
// separate from hud.c because i keep tweaking it independently.

typedef struct {
    glid vao, vbo;
    float size;
    float thickness;
    float r, g, b, a;
    int   dot_mode;   // 0 = cross, 1 = dot
} crosshair;

void crosshair_init(crosshair *ch, float size);
void crosshair_draw(crosshair *ch, glid prog, int sw, int sh);

#endif
