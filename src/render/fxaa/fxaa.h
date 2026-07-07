#ifndef RENDER_FXAA_H
#define RENDER_FXAA_H

#include "../gl.h"
#include "fxaa_programs.h"
#include "fxaa_params.h"
#include "fxaa_quad.h"
#include "fxaa_target.h"
#include "fxaa_blit.h"

// public face of the fxaa post pass. the renderer only includes this. usage:
//
// fxaa f;
// fxaa_init(&f, win_w, win_h);
// ... per frame, after tonemap has written the ldr scene to scene_tex ...
// fxaa_run(&f, scene_tex, /*dst fbo*/ 0, win_w, win_h);
// ... on resize ...
// fxaa_resize(&f, new_w, new_h);
// fxaa_destroy(&f);
//
// run does prepass -> main and writes the antialiased frame into the bound
// destination. if fxaa is disabled or the shaders are missing it does a plain
// blit of scene_tex so the renderer can call it unconditionally and trust the
// frame still lands in `dst`.

typedef struct {
    fxaa_programs prog;
    fxaa_params   params;
    fxaa_quad     quad;
    fxaa_target   target;     // prepass scratch (rgba8, luma in alpha)
    fxaa_blit     blit;       // passthrough fallback
    int  w, h;
    int  ready;               // shaders + target good?
    unsigned frames;          // ran-frame counter for debug
} fxaa;

// build everything for a w x h backbuffer. returns 1 on success. on failure it
// returns 0 but leaves the struct safe to run() (falls back to a blit) and to
// destroy().
int  fxaa_init(fxaa *f, int w, int h);
void fxaa_destroy(fxaa *f);

// resize the scratch target. cheap, early-outs if unchanged.
void fxaa_resize(fxaa *f, int w, int h);

// run the pass. dst is the framebuffer to write into (0 = backbuffer).
void fxaa_run(fxaa *f, glid scene_tex, glid dst, int dst_w, int dst_h);

// mutable params for the debug ui / console.
fxaa_params *fxaa_get_params(fxaa *f);

// is the gpu path actually live (vs. blit fallback)?
int  fxaa_is_active(const fxaa *f);

#endif
