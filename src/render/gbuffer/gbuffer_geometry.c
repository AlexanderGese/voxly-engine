#include "gbuffer_geometry.h"
#include "../../math/mat4.h"

void gbuffer_geometry_begin(gbuffer *g, const texture *atlas) {
    gbuffer_target_bind(&g->target);

    // clear color attachments. the normal target wants to clear to a valid
    // encoded +z normal, not black, otherwise the sky pixels decode to a
    // degenerate direction and the lighting goes weird at the horizon.
    // +z octahedral encodes to (0,0) -> 0.5,0.5 in unorm -> 511/1023.
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glDepthMask(GL_TRUE);
    // opaque geometry only in this pass, transparents come later forward-lit
    glDisable(GL_BLEND);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    glUseProgram(g->prog_geometry);
    gl_set_uniform_mat4(g->prog_geometry, "u_view", mat4_data(&g->view));
    gl_set_uniform_mat4(g->prog_geometry, "u_proj", mat4_data(&g->proj));
    gl_set_uniform_int (g->prog_geometry, "u_atlas", 0);

    // default model is identity until set_model overrides it
    mat4 id = mat4_identity();
    gl_set_uniform_mat4(g->prog_geometry, "u_model", mat4_data(&id));

    texture_bind(atlas, 0);
}

void gbuffer_geometry_set_model(gbuffer *g, mat4 model) {
    gl_set_uniform_mat4(g->prog_geometry, "u_model", mat4_data(&model));
}

void gbuffer_geometry_end(gbuffer *g) {
    (void)g;
    // leave depth attached — the accum/shade passes sample it. just pop the
    // mrt binding so the next pass can target the accum buffer.
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
