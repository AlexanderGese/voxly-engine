#include "bloom2_pass.h"
#include <stddef.h>
static void set_texel_size(glid prog, int w, int h) {
    float tx = (w > 0) ? 1.0f / (float)w : 0.0f;
    float ty = (h > 0) ? 1.0f / (float)h : 0.0f;
    // packed as two floats; the shader declares vec2 u_texel.
    gl_set_uniform_float(prog, "u_texel_x", tx);
    gl_set_uniform_float(prog, "u_texel_y", ty);
}

void bloom2_pass_bright(const bloom2_programs *prog,
                        const bloom2_chain *chain,
                        const bloom2_params *params,
                        const bloom2_quad *quad,
                        glid scene_tex) {
    const bloom2_target *dst = &chain->mip[0];
float knee[4];
bloom2_params_knee_curve(params, knee);
bloom2_target_bind(dst);
glDisable(GL_BLEND);
glUseProgram(prog->bright);
glActiveTexture(GL_TEXTURE0);
glBindTexture(GL_TEXTURE_2D, scene_tex);
gl_set_uniform_int(prog->bright, "u_scene", 0);
gl_set_uniform_float(prog->bright, "u_knee_x", knee[0]);
gl_set_uniform_float(prog->bright, "u_knee_y", knee[1]);
gl_set_uniform_float(prog->bright, "u_knee_z", knee[2]);
gl_set_uniform_float(prog->bright, "u_knee_w", knee[3]);
gl_set_uniform_float(prog->bright, "u_clamp", params->clamp_max);
set_texel_size(prog->bright, dst->w, dst->h);
bloom2_quad_draw(quad);
}

void bloom2_pass_downsample(const bloom2_programs *prog,
                            const bloom2_chain *chain,
                            const bloom2_quad *quad) {
    glDisable(GL_BLEND);
    glUseProgram(prog->down);
    gl_set_uniform_int(prog->down, "u_src", 0);

    // mip[0] already holds the bright pass. reduce into 1, 2, ... count-1.
    for (int i = 1; i < chain->count; i++) {
        const bloom2_target *src = &chain->mip[i - 1];
        const bloom2_target *dst = &chain->mip[i];

        bloom2_target_bind(dst);
        bloom2_target_bind_tex(src, 0);
        // step size is in source texels, so feed the source dims.
        set_texel_size(prog->down, src->w, src->h);

        bloom2_quad_draw(quad);
    }
}

void bloom2_pass_upsample(const bloom2_programs *prog,
                          const bloom2_chain *chain,
                          const bloom2_params *params,
                          const bloom2_tint *tint,
                          const bloom2_quad *quad) {
    // additive: each lower mip adds its tent-filtered glow into the next one
    // up. one-source-one-dest blend keeps the energy bounded.
    glEnable(GL_BLEND);
glBlendFunc(GL_ONE, GL_ONE);
glBlendEquation(GL_FUNC_ADD);
glUseProgram(prog->up);
gl_set_uniform_int(prog->up, "u_src", 0);
gl_set_uniform_float(prog->up, "u_radius", params->radius);
for (int i = chain->count - 1;
i > 0;
i--) {
        const bloom2_target *src = &chain->mip[i];
        const bloom2_target *dst = &chain->mip[i - 1];

        // the *source* mip is the one whose glow we're spreading, so tint by
        // its level — wider mips pick up the far tint.
        vec3 t = bloom2_tint_for_mip(tint, i, chain->count);
        gl_set_uniform_vec3(prog->up, "u_tint", t.x, t.y, t.z);

        bloom2_target_bind(dst);
        bloom2_target_bind_tex(src, 0);
        // tent reads neighbour texels in the *source* mip.
        set_texel_size(prog->up, src->w, src->h);

        bloom2_quad_draw(quad);
    }

    glDisable(GL_BLEND);
