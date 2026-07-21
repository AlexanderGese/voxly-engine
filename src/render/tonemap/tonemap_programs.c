#include "tonemap_programs.h"
#include "../../util/log.h"

#include <stddef.h>

// shares the post passthrough vertex shader with bloom and the rest.
#define TONEMAP_VERT "shaders/post_passthrough.vert"

int tonemap_programs_load(tonemap_programs *p) {
    p->grade = gl_load_shader(TONEMAP_VERT, "shaders/tonemap_grade.frag");
    p->split = gl_load_shader(TONEMAP_VERT, "shaders/tonemap_split.frag");

    if (!p->grade) {
        // split is allowed to be missing; the grade program is not.
        LOGW("tonemap: grade shader missing, pass disabled");
        return 0;
    }
    if (!p->split)
        LOGD("tonemap: split-view shader absent (debug only)");

    LOGI("tonemap: programs loaded");
    return 1;
}

void tonemap_programs_destroy(tonemap_programs *p) {
    if (p->grade) gl_delete_shader(p->grade);
    if (p->split) gl_delete_shader(p->split);
    p->grade = 0;
    p->split = 0;
}

int tonemap_programs_ok(const tonemap_programs *p) {
    return p->grade != 0;
}
