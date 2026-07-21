#include "tonemap_pass.h"

#include <stddef.h>

void tonemap_pass_set_uniforms(glid prog, const tonemap_params *p,
                               float exposure) {
    // curve selection + its scalars
    gl_set_uniform_int(prog, "u_curve", p->curve_kind);
    gl_set_uniform_float(prog, "u_white", p->white);
    gl_set_uniform_float(prog, "u_exposure", exposure);
    gl_set_uniform_float(prog, "u_inv_gamma", 1.0f / p->gamma);
    gl_set_uniform_int(prog, "u_enabled", p->enabled);

    // grade scalars
    const tonemap_grade *g = &p->grade;
    gl_set_uniform_float(prog, "u_contrast", g->contrast);
    gl_set_uniform_float(prog, "u_saturation", g->saturation);

    // white balance resolves the temp/tint knobs into an rgb multiplier so the
    // shader doesnt have to know the mixing constants.
    vec3 wb = tonemap_grade_white_balance(g);
    gl_set_uniform_vec3(prog, "u_white_balance", wb.x, wb.y, wb.z);

    // lift / gamma / gain
    gl_set_uniform_vec3(prog, "u_lift", g->lift.x, g->lift.y, g->lift.z);
    gl_set_uniform_vec3(prog, "u_grade_gamma",
                        g->gamma.x, g->gamma.y, g->gamma.z);
    gl_set_uniform_vec3(prog, "u_gain", g->gain.x, g->gain.y, g->gain.z);

    // middle grey pivot, shared constant so the contrast math matches the cpu
    gl_set_uniform_float(prog, "u_middle_grey", TONEMAP_MIDDLE_GREY);

    // lut: weight is folded with the enable gate so the shader only checks one.
    float lw = (p->lut_enabled ? p->lut_weight : 0.0f);
    gl_set_uniform_float(prog, "u_lut_weight", lw);
}

void tonemap_pass_run(const tonemap_programs *prog,
                      const tonemap_params *p,
                      const tonemap_quad *quad,
                      tonemap_lut *lut,
                      glid scene_tex,
                      float exposure) {
    if (!tonemap_programs_ok(prog)) return;

    // this is a terminal write, never an accumulation. no blend, no depth.
    glDisable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);

    glUseProgram(prog->grade);

    // scene on unit 0
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, scene_tex);
    gl_set_uniform_int(prog->grade, "u_scene", 0);

    // lut on unit 1 (always bind something valid so the sampler is defined; if
    // the weight is 0 the shader wont read it but an unbound sampler3d is ub).
    int have_lut = (lut && p->lut_enabled && lut->dim >= 2);
    if (have_lut) {
        glid t = tonemap_lut_upload(lut);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_3D, t);
    } else {
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_3D, lut ? lut->tex : 0);
    }
    gl_set_uniform_int(prog->grade, "u_lut", 1);
    gl_set_uniform_float(prog->grade, "u_lut_size",
                         lut ? (float)lut->dim : 0.0f);

    tonemap_pass_set_uniforms(prog->grade, p, exposure);

    tonemap_quad_draw(quad);
}
