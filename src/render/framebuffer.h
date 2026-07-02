#ifndef RENDER_FRAMEBUFFER_H
#define RENDER_FRAMEBUFFER_H

#include "gl.h"

// offscreen color + depth framebuffer for future post-processing.
// right now nothing uses it but the infrastructure is here.

typedef struct {
    glid fbo;
    glid color;
    glid depth;
    int  w, h;
} framebuffer;

int  framebuffer_create(framebuffer *fb, int w, int h);
void framebuffer_destroy(framebuffer *fb);
void framebuffer_bind(const framebuffer *fb);
void framebuffer_unbind(void);
void framebuffer_resize(framebuffer *fb, int w, int h);

#endif
