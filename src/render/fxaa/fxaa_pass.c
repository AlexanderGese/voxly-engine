#include "fxaa_pass.h"

// texture units. fxaa never binds more than one source at a time so a single
// unit would do, but keeping a named constant matches the rest of the post
// stack and makes the uniform setup read cleanly.
#define FXAA_UNIT_SCENE   0
#define FXAA_UNIT_SRC     0

void fxaa_pass_prepass(const fxaa_programs *prog,
                       const fxaa_quad *quad,
                       const fxaa_target *dst,
                       glid scene_tex) {
    fxaa_target_bind(dst);
    glUseProgram(prog->prepass);

    glActiveTexture(GL_TEXTURE0 + FXAA_UNIT_SCENE);
    glBindTexture(GL_TEXTURE_2D, scene_tex);
    gl_set_uniform_int(prog->prepass, "u_scene", FXAA_UNIT_SCENE);

    // the prepass owns nothing but the luma weights; hand them down so the
    // shader and fxaa_config.h cant drift apart.
    gl_set_uniform_vec3(prog->prepass, "u_luma_weights",
                        FXAA_LUMA_R, FXAA_LUMA_G, FXAA_LUMA_B);

    // no depth, no blend — straight overwrite of the target.
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
    fxaa_quad_draw(quad);
}

void fxaa_pass_main(const fxaa_programs *prog,
                    const fxaa_quad *quad,
                    const fxaa_target *src,
                    const fxaa_params *params,
                    const float derived[4],
                    int quality_steps,
                    glid out_fbo, int out_w, int out_h) {
    glBindFramebuffer(GL_FRAMEBUFFER, out_fbo);
    glViewport(0, 0, out_w, out_h);
    glUseProgram(prog->main);

    int unit = fxaa_target_bind_tex(src, FXAA_UNIT_SRC);
    gl_set_uniform_int(prog->main, "u_src", unit);

    // 1 / texel size. the neighbour fetches and the edge walk are all in
    // these units; getting it wrong shifts the whole filter by a texel.
    float rcp_x = src->w > 0 ? 1.0f / (float)src->w : 0.0f;
    float rcp_y = src->h > 0 ? 1.0f / (float)src->h : 0.0f;
    gl_set_uniform_vec3(prog->main, "u_rcp_frame", rcp_x, rcp_y, 0.0f);

    // derived scalars: [subpix, subpix^2*0.5, 1/edge_threshold, edge_min].
    gl_set_uniform_float(prog->main, "u_subpix",        derived[0]);
    gl_set_uniform_float(prog->main, "u_subpix_quad",   derived[1]);
    gl_set_uniform_float(prog->main, "u_edge_rcp",      derived[2]);
    gl_set_uniform_float(prog->main, "u_edge_min",      derived[3]);
    gl_set_uniform_float(prog->main, "u_edge_threshold", params->edge_threshold);

    // how many search iterations the shader's loop should run. driven by the
    // quality preset so we dont recompile shaders per quality level.
    gl_set_uniform_int(prog->main, "u_search_steps", quality_steps);
    gl_set_uniform_int(prog->main, "u_show_edges", params->show_edges);

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
    fxaa_quad_draw(quad);
}
