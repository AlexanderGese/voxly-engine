#ifndef RENDER_POST_SSAO_H
#define RENDER_POST_SSAO_H

#include "../gl.h"

// screen-space ambient occlusion stub. not implemented yet but the
// infrastructure is here for when i have time.

typedef struct {
    glid fbo;
    glid tex_occlusion;
    glid tex_noise;
    glid prog;
    glid vao, vbo;
    int  w, h;
    int  kernel_size;
    float radius;
    float bias;
} ssao_pass;

int  ssao_init(ssao_pass *s, int w, int h);
void ssao_destroy(ssao_pass *s);
void ssao_apply(ssao_pass *s, glid depth_tex, glid normal_tex, const float *proj);

#endif
