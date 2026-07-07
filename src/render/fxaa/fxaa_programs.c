#include "fxaa_programs.h"
#include "../../util/log.h"

#include <stddef.h>

// shared post vertex shader, same one bloom2/ssao lean on. lives next to the
// other post shaders in the shaders/ dir at runtime.
#define FXAA_VERT "shaders/post_passthrough.vert"

int fxaa_programs_load(fxaa_programs *p) {
    p->prepass = gl_load_shader(FXAA_VERT, "shaders/fxaa_prepass.frag");
    p->main    = gl_load_shader(FXAA_VERT, "shaders/fxaa_main.frag");

    if (!fxaa_programs_ok(p)) {
        // not fatal. the renderer just blits the scene straight through when
        // fxaa is unavailable. log once so the missing aa is explained.
        LOGW("fxaa: shader load failed (prepass=%u main=%u), pass disabled",
             p->prepass, p->main);
        return 0;
    }
    LOGI("fxaa: programs loaded");
    return 1;
}

void fxaa_programs_destroy(fxaa_programs *p) {
    gl_delete_shader(p->prepass);
    gl_delete_shader(p->main);
    p->prepass = 0;
    p->main = 0;
}

int fxaa_programs_ok(const fxaa_programs *p) {
    return p->prepass && p->main;
}
