#ifndef RENDER_SCREEN_EFFECTS_H
#define RENDER_SCREEN_EFFECTS_H

#include "gl.h"

// fullscreen tint, used for damage flash, underwater blue, vignette, etc.

typedef struct {
    glid prog;
    glid vao;
    glid vbo;
} screen_fx;

int  screen_fx_init(screen_fx *s);
void screen_fx_destroy(screen_fx *s);
void screen_fx_draw_tint(screen_fx *s, float r, float g, float b, float a);

#endif
