#include "gbuffer_shade.h"
#include "gbuffer_debug.h"

void gbuffer_shade_combine(gbuffer *g, float exposure) {
    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);
    glDisable(GL_BLEND);

    glUseProgram(g->prog_shade);

    // albedo from the g-buffer, light from the accumulation buffer
    gbuffer_target_bind_textures(&g->target, 0);
    glActiveTexture(GL_TEXTURE0 + 4);
    glBindTexture(GL_TEXTURE_2D, g->accum_tex);

    gl_set_uniform_int(g->prog_shade, "u_albedo",   0);
    gl_set_uniform_int(g->prog_shade, "u_normal",   1);
    gl_set_uniform_int(g->prog_shade, "u_material", 2);
    gl_set_uniform_int(g->prog_shade, "u_depth",    3);
    gl_set_uniform_int(g->prog_shade, "u_light",    4);

    gl_set_uniform_vec3(g->prog_shade, "u_ambient",
                        g->ambient[0], g->ambient[1], g->ambient[2]);
    gl_set_uniform_float(g->prog_shade, "u_exposure", exposure);

    glBindVertexArray(g->fs_vao);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glBindVertexArray(0);

    glDepthMask(GL_TRUE);
}

void gbuffer_shade(gbuffer *g) {
    if (g->view_mode != GBUFFER_VIEW_FINAL) {
        gbuffer_debug_blit(g, g->view_mode);
        return;
    }
    gbuffer_shade_combine(g, GBUFFER_DEFAULT_EXPOSURE);
}
