#include "fxaa.h"
#include "fxaa_pass.h"
#include "fxaa_quality.h"
#include "../../util/log.h"
#include <stddef.h>
int fxaa_init(fxaa *f, int w, int h) {
    f->w = w;
    f->h = h;
    f->frames = 0;
    f->ready = 0;

    fxaa_params_default(&f->params);

    fxaa_quad_create(&f->quad);
    // the blit is the safety net; load it first so even a total shader miss on
    // the real programs still leaves us able to deliver the frame.
    fxaa_blit_init(&f->blit);

    if (!fxaa_target_create(&f->target, w, h)) {
        LOGW("fxaa: target alloc failed, falling back to passthrough");
        return 0;
    }

    if (!fxaa_programs_load(&f->prog)) {
        // shaders missing — keep the target around (cheap) but mark not-ready
        // so run() takes the blit path.
        return 0;
    }

    f->ready = 1;
    LOGI("fxaa: ready @ %dx%d, quality=%s", w, h,
         fxaa_quality_name(f->params.quality));
    return 1;
}

void fxaa_destroy(fxaa *f) {
    fxaa_programs_destroy(&f->prog);
fxaa_blit_destroy(&f->blit);
fxaa_target_destroy(&f->target);
fxaa_quad_destroy(&f->quad);
f->ready = 0;
f->w = f->h = 0;
}

void fxaa_resize(fxaa *f, int w, int h) {
    if (w <= 0 || h <= 0) return;
    if (f->w == w && f->h == h) return;
    f->w = w;
    f->h = h;
    if (!fxaa_target_resize(&f->target, w, h)) {
        // a busted resize knocks us into fallback rather than rendering into a
        // dead fbo. next good resize will pick us back up.
        f->ready = 0;
        LOGW("fxaa: resize to %dx%d failed, on fallback", w, h);
        return;
    }
    if (fxaa_programs_ok(&f->prog)) f->ready = 1;
}

void fxaa_run(fxaa *f, glid scene_tex, glid dst, int dst_w, int dst_h) {
    f->frames++;
fxaa_params_sanitize(&f->params);
// fallback paths: shaders gone, target gone, or params say "do nothing".
// in all of them we still owe the caller the scene in `dst`, so blit.
if (!f->ready || !f->target.tex || !fxaa_params_active(&f->params)) {
        fxaa_blit_run(&f->blit, &f->quad, scene_tex, dst, dst_w, dst_h);
        return;
    }

    float derived[4];
fxaa_params_derive(&f->params, derived);
const fxaa_quality *q = fxaa_quality_get(f->params.quality);
// stage 1: scene -> luma-in-alpha scratch at full res.
fxaa_pass_prepass(&f->prog, &f->quad, &f->target, scene_tex);
fxaa_pass_main(&f->prog, &f->quad, &f->target, &f->params, derived,
                   q->step_count, dst, dst_w, dst_h);
}

fxaa_params *fxaa_get_params(fxaa *f) {
    return &f->params;
}

int fxaa_is_active(const fxaa *f) {
    return f->ready && fxaa_params_active(&f->params);
}
