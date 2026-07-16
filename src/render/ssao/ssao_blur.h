#ifndef RENDER_SSAO_BLUR_H
#define RENDER_SSAO_BLUR_H
#include "../gl.h"
// the noise-tiling produces a grainy occlusion buffer. a small box blur the
typedef struct {
    glid fbo;
    glid tex;        // blurred occlusion, R8
    glid prog;       // ssaox_blur.frag, 0 if shaders missing
    int  w, h;       // dimensions (matches the occlusion buffer res)
    int  radius;     // box radius per side, in texels
} ssaox_blur;
int  ssaox_blur_init(ssaox_blur *b, int w, int h, glid prog);
void ssaox_blur_destroy(ssaox_blur *b);
void ssaox_blur_resize(ssaox_blur *b, int w, int h);
glid ssaox_blur_run(ssaox_blur *b, glid occl_tex, glid fs_vao);
void ssaox_blur_cpu(const float *in, float *out, int w, int h, int radius);
#endif
