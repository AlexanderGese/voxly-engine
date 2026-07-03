#include "bloom2_programs.h"
#include "../../util/log.h"

#include <stddef.h>

// the vertex shader is shared. it lives alongside the other post shaders.
#define BLOOM2_VERT "shaders/post_passthrough.vert"

int bloom2_programs_load(bloom2_programs *p) {
    p->bright    = gl_load_shader(BLOOM2_VERT, "shaders/bloom2_bright.frag");
    p->down      = gl_load_shader(BLOOM2_VERT, "shaders/bloom2_down.frag");
    p->up        = gl_load_shader(BLOOM2_VERT, "shaders/bloom2_up.frag");
    p->composite = gl_load_shader(BLOOM2_VERT, "shaders/bloom2_composite.frag");

    if (!bloom2_programs_ok(p)) {
        // not fatal — the engine should still render without bloom. log once
        // so its obvious why the glow is missing.
        LOGW("bloom2: one or more shaders missing, post chain disabled");
        return 0;
    }
    LOGI("bloom2: programs loaded");
    return 1;
}

void bloom2_programs_destroy(bloom2_programs *p) {
    gl_delete_shader(p->bright);
    gl_delete_shader(p->down);
    gl_delete_shader(p->up);
    gl_delete_shader(p->composite);
    p->bright = p->down = p->up = p->composite = 0;
}

int bloom2_programs_ok(const bloom2_programs *p) {
    return p->bright && p->down && p->up && p->composite;
}
