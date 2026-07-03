#ifndef RENDER_BLOOM2_TARGET_H
#define RENDER_BLOOM2_TARGET_H

#include "../gl.h"

// a single mip level of the bloom pyramid. one half-float color texture
// wrapped in its own fbo so we can render straight into it. no depth, we
// dont need it for fullscreen passes.

typedef struct {
    glid fbo;
    glid tex;
    int  w, h;
} bloom2_target;

// create a target sized w x h. returns 0 on failure (and leaves the struct
// zeroed so destroy is safe to call anyway).
int  bloom2_target_create(bloom2_target *t, int w, int h);
void bloom2_target_destroy(bloom2_target *t);

// bind for rendering and set the viewport to match. cheap, call per pass.
void bloom2_target_bind(const bloom2_target *t);

// bind the target's color texture to a sampler unit so a later pass can
// read it. returns the unit so the caller can feed it to the uniform.
int  bloom2_target_bind_tex(const bloom2_target *t, int unit);

#endif
