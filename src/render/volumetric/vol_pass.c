#include "vol_pass.h"

#include <stddef.h>

int volumetric_pass_init(volumetric_pass *p, int full_w, int full_h) {
    volumetric_params_defaults(&p->params);
    p->full_w = full_w;
    p->full_h = full_h;
    p->frame = 0;
    p->to_sun = VEC3_UP;
    p->sun_color = vec3_new(1.0f, 1.0f, 1.0f);

    // start with a noon-ish profile so the first frame before set_sun is sane.
    volumetric_medium_profile_from_sun(&p->profile, VEC3_UP,
                                       p->params.g, p->params.scatter,
                                       p->params.extinct);

    // the cpu-side bits never fail: dither matrix, blur kernel, quad.
    volumetric_dither_build(&p->dither);
    volumetric_blur_build(&p->blur);
    if (!volumetric_quad_init(&p->quad)) {
        p->enabled = 0;
        return 0;
    }

    // targets + shaders are the parts that can legitimately fail.
    int tgt_ok = volumetric_target_init(&p->target, full_w, full_h,
                                        p->params.scale);
    int prog_ok = volumetric_programs_load(&p->progs);

    if (tgt_ok && prog_ok) {
        volumetric_dither_upload(&p->dither);
        p->enabled = 1;
    } else {
        // gpu path unavailable — leave the cpu reference usable, disable draw.
        p->enabled = 0;
    }
    return p->enabled;
}

void volumetric_pass_destroy(volumetric_pass *p) {
    volumetric_programs_destroy(&p->progs);
    volumetric_target_destroy(&p->target);
    volumetric_dither_free(&p->dither);
    volumetric_quad_destroy(&p->quad);
    p->enabled = 0;
}

void volumetric_pass_resize(volumetric_pass *p, int full_w, int full_h) {
    if (full_w == p->full_w && full_h == p->full_h) return;
    p->full_w = full_w;
    p->full_h = full_h;
    volumetric_target_resize(&p->target, full_w, full_h);
}

void volumetric_pass_set_sun(volumetric_pass *p, vec3 to_sun, vec3 sun_color) {
    p->to_sun = vec3_normalize(to_sun);
    volumetric_medium_profile_from_sun(&p->profile, to_sun,
                                       p->params.g, p->params.scatter,
                                       p->params.extinct);
    // pre-multiply intensity, strength and the time-of-day tint into the colour
    // the march sees, so the shader just multiplies and accumulates.
    float k = p->params.intensity * p->profile.strength;
    p->sun_color = vec3_mul(vec3_scale(sun_color, k), p->profile.tint);
}

int volumetric_pass_active(const volumetric_pass *p) {
    return p->enabled && p->params.enabled && p->profile.strength > 0.0f;
}

// push the per-frame march uniforms. kept here so run() reads top-to-bottom.
static void upload_march_uniforms(volumetric_pass *p,
                                  mat4 inv_vp, mat4 light_vp) {
    glid prog = p->progs.march;
    gl_set_uniform_mat4(prog, "u_inv_vp",   mat4_data(&inv_vp));
    gl_set_uniform_mat4(prog, "u_light_vp", mat4_data(&light_vp));
    gl_set_uniform_vec3(prog, "u_to_sun",
                        p->to_sun.x, p->to_sun.y, p->to_sun.z);
    gl_set_uniform_vec3(prog, "u_sun_color",
                        p->sun_color.x, p->sun_color.y, p->sun_color.z);
    gl_set_uniform_float(prog, "u_g",        p->profile.medium.g);
    gl_set_uniform_float(prog, "u_scatter",  p->profile.medium.scatter);
    gl_set_uniform_float(prog, "u_extinct",  p->profile.medium.extinct);
    gl_set_uniform_int(prog,   "u_steps",    p->params.steps);
    gl_set_uniform_float(prog, "u_max_dist", p->params.max_dist);

    // dither scale + a per-frame phase so the noise crawls a touch and doesn't
    // look like a fixed screen-door pattern when the camera is still.
    float ds[2];
    volumetric_dither_scale(&p->dither, p->target.w, p->target.h, ds);
    gl_set_uniform_vec3(prog, "u_dither_scale", ds[0], ds[1],
                        (float)(p->frame & 0xff));
}

int volumetric_pass_run(volumetric_pass *p,
                        glid depth_tex, glid shadow_tex,
                        mat4 inv_vp, mat4 light_vp,
                        glid scene_tex,
                        glid out_fbo) {
    if (!volumetric_pass_active(p)) return 0;
    p->frame++;

    // --- 1. march into target slot 0 at reduced res -----------------------
    volumetric_target_bind(&p->target, 0);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);

    volumetric_programs_use_march(&p->progs);
    upload_march_uniforms(p, inv_vp, light_vp);

    glActiveTexture(GL_TEXTURE0 + VOL_TEX_UNIT_DEPTH);
    glBindTexture(GL_TEXTURE_2D, depth_tex);
    glActiveTexture(GL_TEXTURE0 + VOL_TEX_UNIT_SHADOW);
    glBindTexture(GL_TEXTURE_2D, shadow_tex);
    glActiveTexture(GL_TEXTURE0 + VOL_TEX_UNIT_DITHER);
    glBindTexture(GL_TEXTURE_2D, p->dither.tex);

    volumetric_quad_draw(&p->quad);

    // --- 2. separable bilateral blur (slot0 -> slot1 -> slot0) ------------
    volumetric_blur_run(&p->blur, &p->progs, &p->quad, &p->target, depth_tex);

    // --- 3. composite additively over the scene into out_fbo --------------
    glBindFramebuffer(GL_FRAMEBUFFER, out_fbo);
    glViewport(0, 0, p->full_w, p->full_h);

    volumetric_programs_use_composite(&p->progs);
    gl_set_uniform_float(p->progs.composite, "u_intensity", 1.0f);
    glActiveTexture(GL_TEXTURE0 + VOL_TEX_UNIT_SCATTER);
    glBindTexture(GL_TEXTURE_2D, p->target.tex[0]);
    glActiveTexture(GL_TEXTURE0 + VOL_TEX_UNIT_SCENE);
    glBindTexture(GL_TEXTURE_2D, scene_tex);

    // additive: scattered light only ever brightens. the composite shader does
    // the actual add against the sampled scene so we don't need GL blending,
    // but enabling it too is harmless and lets callers feed an empty scene_tex.
    volumetric_quad_draw(&p->quad);

    glEnable(GL_DEPTH_TEST);
    return 1;
}
