#include "tonemap.h"
#include "tonemap_pass.h"
#include "tonemap_cube.h"
#include "../../util/log.h"

#include <stddef.h>

int tonemap_init(tonemap *tm, int w, int h) {
    tm->ready  = 0;
    tm->w      = w;
    tm->h      = h;
    tm->frames = 0;

    tonemap_params_default(&tm->params);
    tonemap_exposure_init(&tm->exposure);
    tonemap_lut_init(&tm->lut);

    tonemap_quad_create(&tm->quad);

    if (!tonemap_programs_load(&tm->prog)) {
        // shaders missing — leave the struct safe but inert. the renderer can
        // still blit the hdr scene with a dumber path.
        LOGW("tonemap: init without shaders, pass will be a no-op");
        return 0;
    }

    tm->ready = 1;
    LOGI("tonemap: ready (%dx%d, curve=%s)", w, h,
         tonemap_curve_name(tm->params.curve_kind));
    return 1;
}

void tonemap_destroy(tonemap *tm) {
    tonemap_programs_destroy(&tm->prog);
    tonemap_quad_destroy(&tm->quad);
    tonemap_lut_destroy(&tm->lut);
    tm->ready = 0;
}

void tonemap_resize(tonemap *tm, int w, int h) {
    if (w == tm->w && h == tm->h) return;
    tm->w = w;
    tm->h = h;
    // nothing resolution-bound to rebuild; the pass works at any size. logged
    // at debug so a resize storm doesnt spam.
    LOGD("tonemap: resized to %dx%d", w, h);
}

void tonemap_feed_luma(tonemap *tm, float avg_luma) {
    tonemap_exposure_measure(&tm->exposure, avg_luma);
}

void tonemap_tick(tonemap *tm, float dt) {
    tonemap_exposure_update(&tm->exposure, dt);
}

void tonemap_run(tonemap *tm, glid scene_tex, glid dst, int dst_w, int dst_h) {
    if (!tm->ready) return;

    // keep params well-formed even if the debug ui has been poking at them.
    tonemap_params_sanitize(&tm->params);

    float exposure = tonemap_exposure_multiplier(&tm->exposure);

    glBindFramebuffer(GL_FRAMEBUFFER, dst);
    glViewport(0, 0, dst_w, dst_h);

    tonemap_pass_run(&tm->prog, &tm->params, &tm->quad, &tm->lut,
                     scene_tex, exposure);

    tm->frames++;
}

int tonemap_load_lut(tonemap *tm, const char *cube_path) {
    int ok = tonemap_cube_load_file(&tm->lut, cube_path);
    // even on a partial/failed parse we got a valid identity, so it's safe to
    // enable. weight defaults to full so the look actually shows.
    tm->params.lut_enabled = 1;
    if (tm->params.lut_weight <= 0.0f)
        tm->params.lut_weight = 1.0f;
    if (ok)
        LOGI("tonemap: lut '%s' active", cube_path);
    return ok;
}

tonemap_params *tonemap_get_params(tonemap *tm) {
    return &tm->params;
}

tonemap_exposure *tonemap_get_exposure(tonemap *tm) {
    return &tm->exposure;
}
