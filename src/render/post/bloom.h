#ifndef RENDER_POST_BLOOM_H
#define RENDER_POST_BLOOM_H
#include "../gl.h"
// simple two-pass bloom: extract bright pixels, blur, composite.
typedef struct {
    glid fbo_bright;
    glid tex_bright;
    glid fbo_blur;
    glid tex_blur;
    glid prog_extract;
    glid prog_blur;
    glid prog_composite;
    glid vao;
    glid vbo;
    int  w, h;
} bloom_pass;
int  bloom_init(bloom_pass *b, int w, int h);
void bloom_destroy(bloom_pass *b);
void bloom_resize(bloom_pass *b, int w, int h);
void bloom_apply(bloom_pass *b, glid scene_tex, float threshold, float intensity);
#endif
