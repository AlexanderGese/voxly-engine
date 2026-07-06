#include "dof_pass.h"
#include "dof_config.h"

void dof_pass_coc(const dof_programs *prog,
                  const dof_quad *quad,
                  glid depth_tex,
                  dof_target *coc_dst,
                  const dof_lens *lens,
                  float znear, float zfar,
                  float texel_scale) {
    if (!prog->ok) return;

    dof_target_bind(coc_dst);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(prog->coc);

    glActiveTexture(GL_TEXTURE0 + DOFX_TEX_UNIT_DEPTH);
    glBindTexture(GL_TEXTURE_2D, depth_tex);

    // the shader reconstructs the same thin-lens coc the cpu reference does,
    // so we hand it the raw lens params rather than a baked curve. keeps the
    // two implementations honest against each other.
    if (prog->u_coc.focus_dist  >= 0) glUniform1f(prog->u_coc.focus_dist,  lens->focus_dist);
    if (prog->u_coc.focal_len   >= 0) glUniform1f(prog->u_coc.focal_len,   lens->focal_len);
    if (prog->u_coc.fstop       >= 0) glUniform1f(prog->u_coc.fstop,       lens->fstop);
    if (prog->u_coc.sensor_w    >= 0) glUniform1f(prog->u_coc.sensor_w,    lens->sensor_w);
    if (prog->u_coc.max_coc     >= 0) glUniform1f(prog->u_coc.max_coc,     lens->max_coc);
    if (prog->u_coc.texel_scale >= 0) glUniform1f(prog->u_coc.texel_scale, texel_scale);

    // pack the near/far planes into the one spare vec2 uniform; the shader
    // needs them to linearize the hardware depth before computing distance.
    if (prog->u_coc.near_far >= 0) glUniform2f(prog->u_coc.near_far, znear, zfar);

    glUseProgram(prog->coc);
    dof_quad_draw(quad);
    glUseProgram(0);
}

void dof_pass_prefilter(const dof_programs *prog,
                        const dof_quad *quad,
                        glid scene_tex,
                        dof_target *color_dst) {
    if (!prog->ok) return;

    // there's no dedicated prefilter program; we lean on the gather program's
    // color sampler with a zero coc, which collapses the gather to a copy and
    // the target's linear filter does the box downsample for free. cheap and
    // it keeps the shader count down.
    dof_target_bind(color_dst);
    glUseProgram(prog->gather);

    glActiveTexture(GL_TEXTURE0 + DOFX_TEX_UNIT_COLOR);
    glBindTexture(GL_TEXTURE_2D, scene_tex);
    if (prog->u_gather.tex_color >= 0)
        glUniform1i(prog->u_gather.tex_color, DOFX_TEX_UNIT_COLOR);

    // force the copy path: zero taps means the gather just emits the centre.
    if (prog->u_gather.tap_count >= 0) glUniform1i(prog->u_gather.tap_count, 0);
    if (prog->u_gather.texel_size >= 0) {
        float tw = (color_dst->w > 0) ? 1.0f / (float)color_dst->w : 0.0f;
        float th = (color_dst->h > 0) ? 1.0f / (float)color_dst->h : 0.0f;
        glUniform2f(prog->u_gather.texel_size, tw, th);
    }

    dof_quad_draw(quad);
    glUseProgram(0);
}
