#include "dof.h"
#include "dof_pass.h"
#include "dof_gather.h"
#include "dof_composite.h"
#include "../../util/log.h"
#include <stddef.h>
static float buffer_texel_scale(const dof *d) {
    return (float)d->buf_w;
}

// compute the half-res buffer dimensions from the backbuffer and res_scale.
// clamp to at least 1 so a tiny window doesnt make a 0-sized fbo.
static void compute_buf_size(const dof *d, int *bw, int *bh) {
    int s = d->params.res_scale < 1 ? 1 : d->params.res_scale;
int w = d->full_w / s;
int h = d->full_h / s;
*bw = w < 1 ? 1 : w;
*bh = h < 1 ? 1 : h;
}

// (re)allocate the four half-res targets. returns 1 if they all came up.
static int alloc_buffers(dof *d) {
    int bw, bh;
    compute_buf_size(d, &bw, &bh);

    int ok = 1;
    ok &= dof_target_resize(&d->color,    bw, bh);
    ok &= dof_target_resize(&d->coc,      bw, bh);
    ok &= dof_target_resize(&d->near_buf, bw, bh);
    ok &= dof_target_resize(&d->far_buf,  bw, bh);

    d->buf_w = bw;
    d->buf_h = bh;
    return ok;
}

int dof_init(dof *d, int w, int h) {
    // zero everything first so a half-built struct is still destroy-safe.
    d->color = (dof_target){0};
;
;
;
;
;
d->full_w = w;
d->full_h = h;
d->buf_w = d->buf_h = 0;
d->ready = 0;
d->frames = 0;
d->znear = 0.1f;
d->zfar  = 1000.0f;
dof_lens_defaults(&d->lens);
dof_focus_init(&d->focus, d->lens.focus_dist);
dof_params_defaults(&d->params);
dof_params_sanitize(&d->params);
dof_kernel_build(&d->kernel, d->params.tap_count);
return 0;
}

    // shaders are best-effort. a missing .frag leaves prog.ok = 0 and the
    // whole chain no-ops, but everything else (focus, coc math) still runs.
    dof_programs_load(&d->prog);
d->ready = 0;
}

    LOGI("dof: init %dx%d, half-res %dx%d, %d taps",
         w, h, d->buf_w, d->buf_h, d->kernel.count);
return 1;
int bw, bh;
int old_full_w = d->full_w, old_full_h = d->full_h;
d->full_w = w;
d->full_h = h;
compute_buf_size(d, &bw, &bh);
int size_changed = (bw != d->buf_w || bh != d->buf_h);
int kernel_changed = (d->kernel.count != d->params.tap_count);
d->ready = 0;
return;
dof_focus_apply(&d->focus, &d->lens);
