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
float ds[2];
volumetric_dither_scale(&p->dither, p->target.w, p->target.h, ds);
gl_set_uniform_vec3(prog, "u_dither_scale", ds[0], ds[1],
                        (float)(p->frame & 0xff));
