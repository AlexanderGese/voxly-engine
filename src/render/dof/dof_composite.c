#include "dof_composite.h"
#include "dof_config.h"

#include <math.h>

static float clamp01(float v) {
    if (v < 0.0f) return 0.0f;
    if (v > 1.0f) return 1.0f;
    return v;
}

static float lerpf(float a, float b, float t) {
    return a + (b - a) * t;
}

float dof_composite_far_weight(float coc_texels, float max_coc) {
    // far field is the positive side of the signed coc. anything negative
    // (near) contributes nothing to the far weight.
    if (coc_texels <= 0.0f || max_coc <= 0.0f) return 0.0f;
    float w = coc_texels / max_coc;
    // smoothstep-ish ease so the transition into blur isnt a hard ramp; a
    // linear weight shows a visible band where it kicks in.
    w = clamp01(w);
    return w * w * (3.0f - 2.0f * w);
}

float dof_composite_near_weight(float coc_texels, float max_coc, float dilate) {
    // near field is the negative side. take magnitude, push it out by the
    // dilation so the bleed starts a touch early, then normalize.
    if (coc_texels >= 0.0f || max_coc <= 0.0f) return 0.0f;
    float mag = -coc_texels + dilate;
    float w = mag / max_coc;
    w = clamp01(w);
    return w * w * (3.0f - 2.0f * w);
}

float dof_composite_blend(float sharp, float near_v, float far_v,
                          float near_w, float far_w, float strength) {
    // far first: blend sharp toward the far blur. this is depth-keyed and
    // never covers foreground, so it's a plain lerp.
    float c = lerpf(sharp, far_v, clamp01(far_w));

    // near on top: near out-of-focus geometry occludes whatever's behind it,
    // so the near blur composites over the result of the far blend, not under.
    c = lerpf(c, near_v, clamp01(near_w));

    // global strength dials the whole effect back toward the original sharp.
    return lerpf(sharp, c, clamp01(strength));
}

void dof_composite_run(const dof_programs *prog,
                       const dof_quad *quad,
                       const dof_target *sharp,
                       const dof_target *near_buf,
                       const dof_target *far_buf,
                       const dof_params *params,
                       int dst_w, int dst_h) {
    if (!prog->ok) return;

    glViewport(0, 0, dst_w, dst_h);
    glUseProgram(prog->composite);

    int u_c = dof_target_bind_tex(sharp,    DOFX_TEX_UNIT_COLOR);
    int u_n = dof_target_bind_tex(near_buf, DOFX_TEX_UNIT_NEAR);
    int u_f = dof_target_bind_tex(far_buf,  DOFX_TEX_UNIT_FAR);
    if (prog->u_comp.tex_color >= 0) glUniform1i(prog->u_comp.tex_color, u_c);
    if (prog->u_comp.tex_near  >= 0) glUniform1i(prog->u_comp.tex_near,  u_n);
    if (prog->u_comp.tex_far   >= 0) glUniform1i(prog->u_comp.tex_far,   u_f);

    if (prog->u_comp.strength >= 0)
        glUniform1f(prog->u_comp.strength, params->strength);
    if (prog->u_comp.near_dilate >= 0)
        glUniform1f(prog->u_comp.near_dilate, params->near_dilate);
    if (prog->u_comp.debug_view >= 0)
        glUniform1i(prog->u_comp.debug_view, params->debug_view);

    dof_quad_draw(quad);
    glUseProgram(0);
}
