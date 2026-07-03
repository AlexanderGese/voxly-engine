#ifndef RENDER_TEXT_H
#define RENDER_TEXT_H

#include "gl.h"
#include "font.h"

// draws ascii text using the bitmap font atlas. uses its own vao/vbo
// filled every frame (no batching, cba).

typedef struct {
    font  f;
    glid  vao;
    glid  vbo;
    glid  prog;
} text_renderer;

int  text_init(text_renderer *t);
void text_destroy(text_renderer *t);
void text_draw(text_renderer *t, const char *s, int x, int y,
               float r, float g, float b, int sw, int sh);

#endif
