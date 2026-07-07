#ifndef RENDER_FXAA_BLIT_H
#define RENDER_FXAA_BLIT_H

#include "../gl.h"
#include "fxaa_quad.h"

// dead-simple passthrough copy. used as the fallback when the fxaa shaders
// fail to load (so fxaa_run still delivers the frame to its destination) and
// as a building block the self-test can lean on. its own tiny program so it
// doesnt depend on the fxaa shaders existing — that would defeat the point.

typedef struct {
    glid prog;
} fxaa_blit;

// load the passthrough program. returns 1 on success. safe to destroy on fail.
int  fxaa_blit_init(fxaa_blit *b);
void fxaa_blit_destroy(fxaa_blit *b);

// copy `src_tex` into the bound framebuffer `dst` at dst_w x dst_h. no-op (and
// returns 0) if the program is missing.
int  fxaa_blit_run(const fxaa_blit *b, const fxaa_quad *quad,
                   glid src_tex, glid dst, int dst_w, int dst_h);

#endif
