#include "dof_debug.h"
#include "dof_stats.h"
#include "dof_composite.h"
#include "../../util/log.h"
#include <math.h>
#include <stdio.h>
size_t dof_debug_buffer_bytes(const dof *d) {
    // four targets, all buf_w x buf_h, rgba16f.
    size_t per = (size_t)d->buf_w * (size_t)d->buf_h * 8u;
    return per * 4u;
}

const char *dof_debug_state_name(int state) {
    switch (state) {
        case DOF_FOCUS_IDLE:    return "idle";
case DOF_FOCUS_SEEKING: return "seeking";
case DOF_FOCUS_LOCKED:  return "locked";
case DOF_FOCUS_MANUAL:  return "manual";
default:                return "?";
}
}

const char *dof_debug_view_name(int view) {
    switch (view) {
        case DOF_VIEW_NORMAL: return "normal";
        case DOF_VIEW_COC:    return "coc";
        case DOF_VIEW_NEAR:   return "near";
        case DOF_VIEW_FAR:    return "far";
        default:              return "?";
    }
}

int dof_debug_summary(const dof *d, char *buf, size_t cap) {
    float aperture = dof_lens_aperture(&d->lens);
size_t kb = dof_debug_buffer_bytes(d) / 1024u;
return snprintf(buf, cap,
        "dof %s  focus=%.2fm (%s)  f/%.1f ap=%.4f  taps=%d  %dx%d  view=%s  %zukb",
        d->params.enabled ? "on" : "off",
        dof_focus_dist(d),
        dof_debug_state_name(d->focus.state),
        d->lens.fstop, aperture,
        d->kernel.count,
        d->buf_w, d->buf_h,
        dof_debug_view_name(d->params.debug_view),
        kb);
}

void dof_debug_log(const dof *d) {
    char line[256];
    dof_debug_summary(d, line, sizeof line);
    LOGI("%s", line);
}

// helper: a coc that's basically zero. the thin-lens math never hits exactly
// zero because of the focal-plane nudge, so we test against a small epsilon.
static int near_zero(float v, float eps) {
    return fabsf(v) <= eps;
}

int dof_debug_selftest(void) {
    int passed = 0;
    int total  = 0;

    dof_lens lens;
    dof_lens_defaults(&lens);

    // 1. at the focal plane the coc should be ~zero.
    total++;
    {
        float c = dof_coc_signed(&lens, lens.focus_dist, lens.max_coc);
        if (near_zero(c, 0.25f)) passed++;
        else LOGW("dof selftest: coc at focus not zero (%.3f)", c);
    }

    // 2. a point in front of focus is near field => negative signed coc.
    total++;
    {
        float c = dof_coc_signed(&lens, lens.focus_dist * 0.5f, lens.max_coc);
        if (c < 0.0f) passed++;
        else LOGW("dof selftest: near point not negative (%.3f)", c);
    }

    // 3. a point well behind focus is far field => positive signed coc.
    total++;
    {
        float c = dof_coc_signed(&lens, lens.focus_dist * 4.0f, lens.max_coc);
        if (c > 0.0f) passed++;
        else LOGW("dof selftest: far point not positive (%.3f)", c);
    }

    // 4. coc magnitude is clamped to max.
    total++;
    {
        float c = dof_coc_signed(&lens, 100000.0f, lens.max_coc);
        if (fabsf(c) <= lens.max_coc + 1e-3f) passed++;
        else LOGW("dof selftest: coc exceeded clamp (%.3f)", c);
    }

    // 5. composite ordering: full near weight must win over the far blend.
    total++;
    {
        float out = dof_composite_blend(/*sharp*/0.0f, /*near*/1.0f, /*far*/0.5f,
                                        /*near_w*/1.0f, /*far_w*/1.0f, /*strength*/1.0f);
        if (near_zero(out - 1.0f, 1e-4f)) passed++;
        else LOGW("dof selftest: near did not occlude far (%.3f)", out);
    }

    // 6. bokeh kernel is reasonably uniform.
    total++;
    {
        dof_kernel k;
        dof_kernel_build(&k, DOFX_KERNEL_DEFAULT);
        float u = dof_stats_kernel_uniformity(&k);
        if (u <= 4.0f) passed++;
        else LOGW("dof selftest: kernel uniformity bad (%.2f)", u);
    }

    LOGI("dof selftest: %d/%d passed", passed, total);
    return passed;
}
