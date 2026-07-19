#include "ssr_pass.h"
#include "../../util/log.h"

#include <stddef.h>

static int divup_scale(int v, int scale) {
    int r = v / scale;
    return r < 1 ? 1 : r;
}

// half-res reflection target: rgb reflected color + alpha confidence. needs
// float-ish precision because the lit color is hdr-ish before tonemap, so f16.
static glid make_reflect_tex(int w, int h) {
    glid t;
    glGenTextures(1, &t);
    glBindTexture(GL_TEXTURE_2D, t);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, w, h, 0, GL_RGBA, GL_FLOAT, NULL);
    // linear so the upsample in resolve is smooth — half-res ssr leans on it.
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_2D, 0);
    return t;
}

static glid make_result_tex(int w, int h) {
    glid t;
    glGenTextures(1, &t);
    glBindTexture(GL_TEXTURE_2D, t);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, w, h, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_2D, 0);
    return t;
}

static void attach(glid fbo, glid tex) {
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                           GL_TEXTURE_2D, tex, 0);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        LOGW("ssrx fbo incomplete");
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

int ssrx_pass_init(ssrx_pass *p, int full_w, int full_h) {
    ssrx_settings_default(&p->settings);
    ssrx_settings_clamp(&p->settings);
    ssrx_stats_reset(&p->stats);

    p->full_w = full_w;
    p->full_h = full_h;
    p->w = divup_scale(full_w, p->settings.scale);
    p->h = divup_scale(full_h, p->settings.scale);

    // reflect target (half res)
    p->tex_reflect = make_reflect_tex(p->w, p->h);
    glGenFramebuffers(1, &p->fbo_reflect);
    attach(p->fbo_reflect, p->tex_reflect);

    // resolve target (full res)
    p->tex_result = make_result_tex(full_w, full_h);
    glGenFramebuffers(1, &p->fbo_resolve);
    attach(p->fbo_resolve, p->tex_result);

    ssrx_fsquad_init(&p->quad);

    p->prog_reflect = gl_load_shader(SSRX_VERT_PATH, SSRX_FRAG_REFLECT_PATH);
    p->prog_resolve = gl_load_shader(SSRX_VERT_PATH, SSRX_FRAG_RESOLVE_PATH);
    p->prog_blur    = gl_load_shader(SSRX_VERT_PATH, SSRX_FRAG_BLUR_PATH);

    // blur runs at the reflect (half) res and falls back to a pass-through if
    // its shader is missing — it isnt load-bearing, just prettier.
    ssrx_blur_init(&p->blur, p->w, p->h, p->prog_blur, SSRX_BLUR_MAX_RADIUS);

    p->enabled = (p->prog_reflect != 0 && p->prog_resolve != 0);
    if (!p->enabled) {
        LOGW("ssrx: shaders missing — gpu pass disabled (cpu reference still ok)");
    } else {
        LOGI("ssrx: ready, reflect @ %dx%d (1/%d res)", p->w, p->h, p->settings.scale);
    }
    return p->enabled;
}

void ssrx_pass_destroy(ssrx_pass *p) {
    ssrx_blur_destroy(&p->blur);
    ssrx_fsquad_destroy(&p->quad);
    if (p->fbo_reflect) glDeleteFramebuffers(1, &p->fbo_reflect);
    if (p->fbo_resolve) glDeleteFramebuffers(1, &p->fbo_resolve);
    if (p->tex_reflect) glDeleteTextures(1, &p->tex_reflect);
    if (p->tex_result)  glDeleteTextures(1, &p->tex_result);
    gl_delete_shader(p->prog_reflect);
    gl_delete_shader(p->prog_resolve);
    gl_delete_shader(p->prog_blur);
    p->fbo_reflect = p->fbo_resolve = 0;
    p->tex_reflect = p->tex_result  = 0;
    p->prog_reflect = p->prog_resolve = p->prog_blur = 0;
}

void ssrx_pass_resize(ssrx_pass *p, int full_w, int full_h) {
    if (p->full_w == full_w && p->full_h == full_h) return;
    p->full_w = full_w;
    p->full_h = full_h;
    p->w = divup_scale(full_w, p->settings.scale);
    p->h = divup_scale(full_h, p->settings.scale);

    if (p->tex_reflect) glDeleteTextures(1, &p->tex_reflect);
    p->tex_reflect = make_reflect_tex(p->w, p->h);
    attach(p->fbo_reflect, p->tex_reflect);

    if (p->tex_result) glDeleteTextures(1, &p->tex_result);
    p->tex_result = make_result_tex(full_w, full_h);
    attach(p->fbo_resolve, p->tex_result);

    ssrx_blur_resize(&p->blur, p->w, p->h);
}

void ssrx_pass_apply_scale(ssrx_pass *p) {
    ssrx_settings_clamp(&p->settings);
    int nw = divup_scale(p->full_w, p->settings.scale);
    int nh = divup_scale(p->full_h, p->settings.scale);
    if (nw == p->w && nh == p->h) return;   // unchanged
    p->w = nw;
    p->h = nh;
    if (p->tex_reflect) glDeleteTextures(1, &p->tex_reflect);
    p->tex_reflect = make_reflect_tex(p->w, p->h);
    attach(p->fbo_reflect, p->tex_reflect);
    ssrx_blur_resize(&p->blur, p->w, p->h);
    LOGI("ssrx: reflect buffer now %dx%d (1/%d res)", p->w, p->h, p->settings.scale);
}

// push the march/resolve tunables to the reflect shader as uniforms.
static void upload_reflect_uniforms(ssrx_pass *p, const ssrx_gbuffer *g) {
    const ssrx_settings *s = &p->settings;
    gl_set_uniform_mat4(p->prog_reflect, "u_proj", mat4_data(&g->proj));
    gl_set_uniform_mat4(p->prog_reflect, "u_inv_proj", mat4_data(&g->inv_proj));

    gl_set_uniform_int  (p->prog_reflect, "u_max_steps",    s->max_steps);
    gl_set_uniform_int  (p->prog_reflect, "u_refine_steps", s->refine_steps);
    gl_set_uniform_float(p->prog_reflect, "u_stride",       s->stride);
    gl_set_uniform_float(p->prog_reflect, "u_growth",       s->growth);
    gl_set_uniform_float(p->prog_reflect, "u_thickness",    s->thickness);

    gl_set_uniform_float(p->prog_reflect, "u_strength",     s->strength);
    gl_set_uniform_float(p->prog_reflect, "u_edge_fade",    s->edge_fade);
    gl_set_uniform_float(p->prog_reflect, "u_dist_fade",    s->dist_fade);
    gl_set_uniform_float(p->prog_reflect, "u_rough_cutoff", s->rough_cutoff);
    gl_set_uniform_float(p->prog_reflect, "u_base_f0",      s->base_f0);
}

static void bind_gbuffer_inputs(ssrx_pass *p, const ssrx_gbuffer *g) {
    glActiveTexture(GL_TEXTURE0 + SSRX_TEX_UNIT_DEPTH);
    glBindTexture(GL_TEXTURE_2D, g->tex_depth);
    gl_set_uniform_int(p->prog_reflect, "u_depth", SSRX_TEX_UNIT_DEPTH);

    glActiveTexture(GL_TEXTURE0 + SSRX_TEX_UNIT_NORMAL);
    glBindTexture(GL_TEXTURE_2D, g->tex_normal);
    gl_set_uniform_int(p->prog_reflect, "u_normal", SSRX_TEX_UNIT_NORMAL);

    glActiveTexture(GL_TEXTURE0 + SSRX_TEX_UNIT_COLOR);
    glBindTexture(GL_TEXTURE_2D, g->tex_color);
    gl_set_uniform_int(p->prog_reflect, "u_color", SSRX_TEX_UNIT_COLOR);

    // material is optional; bind whatever we have and tell the shader.
    glActiveTexture(GL_TEXTURE0 + SSRX_TEX_UNIT_MATERIAL);
    glBindTexture(GL_TEXTURE_2D, g->tex_material);
    gl_set_uniform_int(p->prog_reflect, "u_material", SSRX_TEX_UNIT_MATERIAL);
    gl_set_uniform_int(p->prog_reflect, "u_has_material", g->tex_material ? 1 : 0);
}

glid ssrx_pass_run(ssrx_pass *p, const ssrx_gbuffer *g, glid scene_color) {
    if (!p->enabled || !p->settings.enabled) return scene_color;
    if (!ssrx_gbuffer_valid(g)) return scene_color;

    ssrx_pass_apply_scale(p);

    // ---- reflect stage: march, write half-res rgb+weight ----
    glBindFramebuffer(GL_FRAMEBUFFER, p->fbo_reflect);
    glViewport(0, 0, p->w, p->h);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);   // weight 0 = no reflection
    glClear(GL_COLOR_BUFFER_BIT);
    glUseProgram(p->prog_reflect);
    bind_gbuffer_inputs(p, g);
    upload_reflect_uniforms(p, g);
    ssrx_fsquad_draw(&p->quad);

    // ---- blur stage: spread the reflection by surface roughness ----
    // returns the blurred tex, or tex_reflect untouched if the shader is gone.
    glid reflect_src = ssrx_blur_run(&p->blur, p->tex_reflect,
                                     g->tex_material, p->quad.vao);

    // ---- resolve stage: composite reflection over the full-res scene ----
    glBindFramebuffer(GL_FRAMEBUFFER, p->fbo_resolve);
    glViewport(0, 0, p->full_w, p->full_h);
    glUseProgram(p->prog_resolve);

    glActiveTexture(GL_TEXTURE0 + SSRX_TEX_UNIT_SCENE);
    glBindTexture(GL_TEXTURE_2D, scene_color);
    gl_set_uniform_int(p->prog_resolve, "u_scene", SSRX_TEX_UNIT_SCENE);

    glActiveTexture(GL_TEXTURE0 + SSRX_TEX_UNIT_REFLECT);
    glBindTexture(GL_TEXTURE_2D, reflect_src);
    gl_set_uniform_int(p->prog_resolve, "u_reflect", SSRX_TEX_UNIT_REFLECT);
    gl_set_uniform_float(p->prog_resolve, "u_strength", p->settings.strength);

    ssrx_fsquad_draw(&p->quad);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    return p->tex_result;
}

glid ssrx_pass_result(const ssrx_pass *p) {
    return p->tex_result;
}
