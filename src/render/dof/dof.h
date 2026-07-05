#ifndef RENDER_DOF_H
#define RENDER_DOF_H
#include "../gl.h"
#include "dof_coc.h"
#include "dof_focus.h"
#include "dof_kernel.h"
#include "dof_params.h"
#include "dof_programs.h"
#include "dof_target.h"
#include "dof_quad.h"
// public face of the depth-of-field post chain. the renderer only needs this
// header. lifecycle mirrors the bloom2 module so it slots into the same place:
//
// dof d;
// dof_init(&d, win_w, win_h);
// ... per frame, after the hdr scene + depth are available ...
// dof_set_autofocus(&d, depth_under_reticle, dt);   // or set_focus_manual
// dof_run(&d, scene_tex, depth_tex, /*dst fbo*/ 0, win_w, win_h);
// ... on resize ...
// dof_resize(&d, new_w, new_h);
// dof_destroy(&d);
//
// dof_run renders the composited (sharp + bokeh) image into `dst`. if the
// shaders didnt load or the effect is disabled it does nothing and the caller
typedef struct {
    dof_lens      lens;      // the optics
    dof_focus     focus;     // autofocus driver
    dof_kernel    kernel;    // bokeh disk taps
    dof_params    params;    // runtime knobs
    dof_programs  prog;      // gl programs + uniforms
    dof_quad      quad;      // fullscreen tri

    // half-res buffers. color is the prefiltered scene; coc the signed coc;
    // near/far the two gather results.
    dof_target    color;
    dof_target    coc;
    dof_target    near_buf;
    dof_target    far_buf;

    int  full_w, full_h;     // backbuffer size we were built for
    int  buf_w, buf_h;       // half-res buffer size (full / res_scale)
    int  ready;              // 1 if buffers + shaders are good
    unsigned frames;         // ran-frame counter for debug

    float znear, zfar;       // camera planes, needed to linearize depth
} dof;
int  dof_init(dof *d, int w, int h);
void dof_destroy(dof *d);
void dof_resize(dof *d, int w, int h);
void dof_set_planes(dof *d, float znear, float zfar);
void dof_set_autofocus(dof *d, float reticle_depth, float dt);
void dof_set_focus_manual(dof *d, float dist);
void dof_resume_autofocus(dof *d);
void dof_run(dof *d, glid scene_tex, glid depth_tex, glid dst, int dst_w, int dst_h);
dof_params *dof_get_params(dof *d);
float dof_focus_dist(const dof *d);
#endif
