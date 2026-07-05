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
// is expected to have the sharp scene already in `dst`.

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

// build everything for a w x h backbuffer. returns 1 on success. on failure it
// returns 0 but leaves the struct safe to destroy and to call run() on (run
// becomes a no-op). shaders missing => not ready but not an error.
int  dof_init(dof *d, int w, int h);
void dof_destroy(dof *d);

// rebuild buffers for a new backbuffer size. cheap to call every frame; it
// early-outs when nothing changed. also picks up res_scale / tap_count edits.
void dof_resize(dof *d, int w, int h);

// set the camera planes used to linearize the hardware depth buffer.
void dof_set_planes(dof *d, float znear, float zfar);

// feed the autofocus a depth reading (e.g. the depth sampled under the
// crosshair). pass <=0 for "no reading". updates the lens focus distance.
void dof_set_autofocus(dof *d, float reticle_depth, float dt);

// pin the focus distance, disabling autofocus until dof_resume_autofocus.
void dof_set_focus_manual(dof *d, float dist);
void dof_resume_autofocus(dof *d);

// run the full chain (coc -> prefilter -> gather near/far -> composite) into
// `dst`. no-op if disabled or not ready.
void dof_run(dof *d, glid scene_tex, glid depth_tex, glid dst, int dst_w, int dst_h);

// mutable params for the debug ui. changes to res/taps take effect on the next
// dof_resize (which dof_run does NOT call for you).
dof_params *dof_get_params(dof *d);

// the live focus distance, for the hud readout.
float dof_focus_dist(const dof *d);

#endif
