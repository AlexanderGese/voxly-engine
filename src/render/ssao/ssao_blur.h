#ifndef RENDER_SSAO_BLUR_H
#define RENDER_SSAO_BLUR_H

#include "../gl.h"

// the noise-tiling produces a grainy occlusion buffer. a small box blur the
// size of the noise tile (default 4x4) smears the pattern out completely and
// is basically free. we do it as a single NxN box in the shader rather than
// two separable passes — the kernel is tiny and a second fbo bounce isnt
// worth it at this size.
//
// this struct owns its own output fbo+texture (single channel R8).

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

// blur `occl_tex` into b->tex. binds a fullscreen-triangle vao the caller
// already has bound? no — caller passes the vao to keep this self contained.
// returns b->tex (the blurred result) for convenience.
glid ssaox_blur_run(ssaox_blur *b, glid occl_tex, glid fs_vao);

// cpu reference box blur over a single-channel float buffer. used by tests
// and the cpu compute path. in/out are w*h, out may alias in is NOT allowed.
void ssaox_blur_cpu(const float *in, float *out, int w, int h, int radius);

#endif
