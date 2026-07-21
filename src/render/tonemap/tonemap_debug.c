#include "tonemap_debug.h"
#include "tonemap_pass.h"
#include "../../util/log.h"

#include <stdio.h>

int tonemap_debug_summary(const tonemap *tm, char *buf, size_t cap) {
    const tonemap_params *p = &tm->params;
    const tonemap_exposure *e = &tm->exposure;

    // effective ev (manual + adapted) is what people actually want to read off.
    float ev   = tonemap_exposure_ev(e);
    float mult = tonemap_exposure_multiplier(e);

    return snprintf(buf, cap,
        "tonemap: %s | ev %+.2f (x%.2f) %s | con %.2f sat %.2f "
        "wb %+.2f/%+.2f | lut %s %.0f%% | %s",
        tonemap_curve_name(p->curve_kind),
        ev, mult, e->auto_enabled ? "auto" : "manual",
        p->grade.contrast, p->grade.saturation,
        p->grade.temperature, p->grade.tint,
        p->lut_enabled ? "on" : "off", p->lut_weight * 100.0f,
        p->enabled ? "active" : "BYPASS");
}

void tonemap_debug_log(const tonemap *tm) {
    char line[256];
    tonemap_debug_summary(tm, line, sizeof line);
    LOGI("%s", line);
}

void tonemap_debug_run_split(const tonemap *tm, glid scene_tex,
                             glid dst, int dst_w, int dst_h, float seam) {
    if (!tm->ready || !tm->prog.split) return;
    if (seam < 0.0f) seam = 0.0f;
    if (seam > 1.0f) seam = 1.0f;

    const tonemap_params *p = &tm->params;
    float exposure = tonemap_exposure_multiplier(&tm->exposure);

    glBindFramebuffer(GL_FRAMEBUFFER, dst);
    glViewport(0, 0, dst_w, dst_h);
    glDisable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);

    glUseProgram(tm->prog.split);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, scene_tex);
    gl_set_uniform_int(tm->prog.split, "u_scene", 0);

    gl_set_uniform_int(tm->prog.split, "u_curve", p->curve_kind);
    gl_set_uniform_float(tm->prog.split, "u_white", p->white);
    gl_set_uniform_float(tm->prog.split, "u_exposure", exposure);
    gl_set_uniform_float(tm->prog.split, "u_inv_gamma", 1.0f / p->gamma);
    gl_set_uniform_float(tm->prog.split, "u_contrast", p->grade.contrast);
    gl_set_uniform_float(tm->prog.split, "u_saturation", p->grade.saturation);

    vec3 wb = tonemap_grade_white_balance(&p->grade);
    gl_set_uniform_vec3(tm->prog.split, "u_white_balance", wb.x, wb.y, wb.z);
    gl_set_uniform_float(tm->prog.split, "u_middle_grey", TONEMAP_MIDDLE_GREY);
    gl_set_uniform_float(tm->prog.split, "u_split", seam);

    tonemap_quad_draw(&tm->quad);
}

int tonemap_debug_cycle_curve(tonemap *tm) {
    int k = tm->params.curve_kind + 1;
    if (k >= TONEMAP_CURVE_COUNT) k = 0;
    tm->params.curve_kind = k;
    LOGD("tonemap: curve -> %s", tonemap_curve_name(k));
    return k;
}
