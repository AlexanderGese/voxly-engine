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
int  fxaa_init(fxaa *f, int w, int h);
void fxaa_destroy(fxaa *f);
void fxaa_resize(fxaa *f, int w, int h);
void fxaa_run(fxaa *f, glid scene_tex, glid dst, int dst_w, int dst_h);
fxaa_params *fxaa_get_params(fxaa *f);
int  fxaa_is_active(const fxaa *f);
#endif
