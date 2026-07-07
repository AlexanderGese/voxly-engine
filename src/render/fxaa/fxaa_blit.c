#include "fxaa_blit.h"
#include "../../util/log.h"

#include <stddef.h>

#define FXAA_BLIT_VERT "shaders/post_passthrough.vert"

int fxaa_blit_init(fxaa_blit *b) {
    b->prog = gl_load_shader(FXAA_BLIT_VERT, "shaders/fxaa_blit.frag");
    if (!b->prog) {
        // if even the passthrough is gone the renderer has bigger problems,
        // but dont crash — just refuse to draw.
        LOGW("fxaa: blit shader missing, fallback copy unavailable");
        return 0;
    }
    return 1;
}

void fxaa_blit_destroy(fxaa_blit *b) {
    gl_delete_shader(b->prog);
    b->prog = 0;
}

int fxaa_blit_run(const fxaa_blit *b, const fxaa_quad *quad,
                  glid src_tex, glid dst, int dst_w, int dst_h) {
    if (!b->prog) return 0;

    glBindFramebuffer(GL_FRAMEBUFFER, dst);
    glViewport(0, 0, dst_w, dst_h);
    glUseProgram(b->prog);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, src_tex);
    gl_set_uniform_int(b->prog, "u_src", 0);

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
    fxaa_quad_draw(quad);
    return 1;
}
