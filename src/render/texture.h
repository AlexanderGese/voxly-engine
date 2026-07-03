#ifndef RENDER_TEXTURE_H
#define RENDER_TEXTURE_H

#include "gl.h"

// loads a texture from disk (png/jpg via stb_image).
// filtering is nearest by default (for that crisp pixel look)

typedef struct {
    glid id;
    int  w;
    int  h;
} texture;

int  texture_load(texture *t, const char *path);
void texture_bind(const texture *t, int unit);
void texture_destroy(texture *t);

#endif
