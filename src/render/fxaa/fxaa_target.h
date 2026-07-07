#ifndef RENDER_FXAA_TARGET_H
#define RENDER_FXAA_TARGET_H

#include "../gl.h"

// the intermediate target fxaa renders the luma prepass into. one rgba8
// texture: rgb is the (tonemapped, ldr) scene color copied through, alpha is
// the packed luma the main pass reads. linear filtering is mandatory — the
// edge search relies on hardware bilinear taps landing between texels.

typedef struct {
    glid fbo;
    glid tex;     // rgba8, .a = luma
    int  w, h;
} fxaa_target;

// create a target sized w x h. zeroes the struct first so destroy is always
// safe. returns 0 on failure.
int  fxaa_target_create(fxaa_target *t, int w, int h);
void fxaa_target_destroy(fxaa_target *t);

// resize in place. no-op if the size is unchanged. returns 1 if the target is
// usable afterwards.
int  fxaa_target_resize(fxaa_target *t, int w, int h);

// bind for rendering, viewport set to match.
void fxaa_target_bind(const fxaa_target *t);

// bind the color/luma texture to a sampler unit. returns the unit.
int  fxaa_target_bind_tex(const fxaa_target *t, int unit);

#endif
