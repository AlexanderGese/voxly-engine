#include "dof.h"
#include "dof_pass.h"
#include "dof_gather.h"
#include "dof_composite.h"
#include "../../util/log.h"

#include <stddef.h>

// the texel scale converts a sensor-space coc (a fraction of the frame) into
// texels of the half-res buffer. it's just the buffer width; the coc shader
// multiplies the normalized coc by this.
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
    d->coc = (dof_target){0};
    d->near_buf = (dof_target){0};
    d->far_buf = (dof_target){0};
    d->prog = (dof_programs){0};
    d->quad = (dof_quad){0};
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

    if (!dof_quad_create(&d->quad)) {
        LOGE("dof: failed to create fullscreen quad");
        return 0;
    }

    if (!alloc_buffers(d)) {
        LOGE("dof: failed to allocate gather buffers");
        return 0;
    }

    // shaders are best-effort. a missing .frag leaves prog.ok = 0 and the
    // whole chain no-ops, but everything else (focus, coc math) still runs.
    dof_programs_load(&d->prog);
    if (d->prog.ok) {
        dof_programs_upload_kernel(&d->prog, &d->kernel);
        d->ready = 1;
    } else {
        LOGW("dof: initialized without gpu path (shaders missing)");
        d->ready = 0;
    }

    LOGI("dof: init %dx%d, half-res %dx%d, %d taps",
         w, h, d->buf_w, d->buf_h, d->kernel.count);
    return 1;
}

void dof_destroy(dof *d) {
    dof_programs_unload(&d->prog);
    dof_quad_destroy(&d->quad);
    dof_target_destroy(&d->color);
    dof_target_destroy(&d->coc);
    dof_target_destroy(&d->near_buf);
    dof_target_destroy(&d->far_buf);
    d->ready = 0;
}

void dof_resize(dof *d, int w, int h) {
    dof_params_sanitize(&d->params);

    int bw, bh;
    // figure the target buffer size for the (possibly new) backbuffer + scale.
    int old_full_w = d->full_w, old_full_h = d->full_h;
    d->full_w = w;
    d->full_h = h;
    compute_buf_size(d, &bw, &bh);

    int size_changed = (bw != d->buf_w || bh != d->buf_h);
    int kernel_changed = (d->kernel.count != d->params.tap_count);

    if (!size_changed && !kernel_changed &&
        old_full_w == w && old_full_h == h) {
        return; // genuinely nothing to do
    }

    if (size_changed) {
        if (!alloc_buffers(d)) {
            LOGE("dof: resize failed, disabling");
            d->ready = 0;
            return;
        }
    }

    if (kernel_changed) {
        dof_kernel_build(&d->kernel, d->params.tap_count);
        if (d->prog.ok) dof_programs_upload_kernel(&d->prog, &d->kernel);
    }

    d->ready = d->prog.ok ? 1 : 0;
}

void dof_set_planes(dof *d, float znear, float zfar) {
    if (znear > 0.0f) d->znear = znear;
    if (zfar > znear)  d->zfar  = zfar;
}

void dof_set_autofocus(dof *d, float reticle_depth, float dt) {
    dof_focus_feed(&d->focus, reticle_depth, dt);
    dof_focus_apply(&d->focus, &d->lens);
}

void dof_set_focus_manual(dof *d, float dist) {
    dof_focus_set_manual(&d->focus, dist);
    dof_focus_apply(&d->focus, &d->lens);
}

void dof_resume_autofocus(dof *d) {
    dof_focus_resume_auto(&d->focus);
}

void dof_run(dof *d, glid scene_tex, glid depth_tex, glid dst, int dst_w, int dst_h) {
    if (!d->ready || !d->params.enabled) return;
    d->frames++;

    // save the gl state we trample so we hand the caller back a clean slate.
    // the chain wants depth test off (fullscreen passes) and no blending.
    GLboolean had_depth = glIsEnabled(GL_DEPTH_TEST);
    GLboolean had_blend = glIsEnabled(GL_BLEND);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);

    float tscale = buffer_texel_scale(d);

    // 1. coc from depth into the coc buffer.
    dof_pass_coc(&d->prog, &d->quad, depth_tex, &d->coc,
                 &d->lens, d->znear, d->zfar, tscale);

    // 2. prefilter / downsample the scene into the half-res color buffer. this
    // restores the real tap count the prefilter clobbered to 0.
    dof_pass_prefilter(&d->prog, &d->quad, scene_tex, &d->color);
    if (d->prog.gather && d->prog.u_gather.tap_count >= 0) {
        glUseProgram(d->prog.gather);
        glUniform1i(d->prog.u_gather.tap_count, d->kernel.count);
        glUseProgram(0);
    }

    // 3. gather near + far.
    dof_gather_run(&d->prog, &d->quad, &d->color, &d->coc,
                   &d->near_buf, &d->far_buf);

    // 4. composite into dst at full res.
    glBindFramebuffer(GL_FRAMEBUFFER, dst);
    // the sharp source for the composite is the original full-res scene; we
    // wrap it in a throwaway target view so the composite's bind_tex helper
    // works uniformly. (cheap: no allocation, just a struct on the stack.)
    dof_target sharp_view;
    sharp_view.fbo = 0;
    sharp_view.tex = scene_tex;
    sharp_view.w = dst_w;
    sharp_view.h = dst_h;
    dof_composite_run(&d->prog, &d->quad, &sharp_view,
                      &d->near_buf, &d->far_buf, &d->params, dst_w, dst_h);

    // restore state.
    if (had_depth) glEnable(GL_DEPTH_TEST);
    if (had_blend) glEnable(GL_BLEND);
    GL_CHECK();
}

dof_params *dof_get_params(dof *d) {
    return &d->params;
}

float dof_focus_dist(const dof *d) {
    return dof_focus_distance(&d->focus);
}
