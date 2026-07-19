#ifndef RENDER_SSR_PASS_H
#define RENDER_SSR_PASS_H

#include "ssr_config.h"
#include "ssr_gbuffer.h"
#include "ssr_settings.h"
#include "ssr_fsquad.h"
#include "ssr_stats.h"
#include "ssr_blur.h"

// the gpu ssr pass. two stages:
//
// reflect  — march the depth buffer, write a half-res RGBA reflection buffer:
// rgb = reflected scene color, a = confidence weight. this is the
// expensive one (the fill).
// resolve  — composite the (upsampled) reflection over the full-res scene
// using its alpha as the blend weight. cheap.
//
// the pass owns its fbos/textures/shaders. feed it a gbuffer + the lit scene
// color each frame; it leaves the composited result in tex_result and fbo 0
// bound. if the shaders fail to load the pass disables itself and run() is a
// no-op (the cpu reference path under ssr_march/ssr_resolve still works for
// tests regardless).

typedef struct {
    // reflect stage
    glid fbo_reflect;
    glid tex_reflect;     // RGBA16F, half-res, rgb=color a=weight
    glid prog_reflect;

    // roughness blur stage (between reflect and resolve)
    ssrx_blur     blur;
    glid          prog_blur;

    // resolve stage
    glid fbo_resolve;
    glid tex_result;      // RGBA, full-res composited output
    glid prog_resolve;

    ssrx_fsquad   quad;
    ssrx_settings settings;
    ssrx_stats    stats;

    int full_w, full_h;   // scene resolution
    int w, h;             // reflect buffer resolution (full / scale)

    int enabled;          // 0 if shaders missing
} ssrx_pass;

// init at the scene resolution. builds buffers at full/scale. returns 1 if the
// gpu path is live, 0 if disabled (still safe to call run/destroy).
int  ssrx_pass_init(ssrx_pass *p, int full_w, int full_h);
void ssrx_pass_destroy(ssrx_pass *p);

// resize to a new scene resolution. cheap no-op if unchanged.
void ssrx_pass_resize(ssrx_pass *p, int full_w, int full_h);

// rebuild the reflect buffer at the current settings.scale (after the debug ui
// changes the downsample factor). safe to call every frame; only acts on change.
void ssrx_pass_apply_scale(ssrx_pass *p);

// run both stages. `scene_color` is the full-res lit scene we reflect and
// composite over. returns tex_result, or scene_color unchanged if disabled.
glid ssrx_pass_run(ssrx_pass *p, const ssrx_gbuffer *g, glid scene_color);

// the composited texture from the last run.
glid ssrx_pass_result(const ssrx_pass *p);

#endif
