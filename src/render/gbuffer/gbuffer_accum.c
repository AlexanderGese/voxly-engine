#include "gbuffer_accum.h"
#include "../../util/log.h"

#include <stdio.h>

// helper: set a "u_light_xxx[i]" style uniform without sprintf'ing all over
// the call sites. the gl wrapper looks up by name each time which isn't free
// but the batch is small and this only runs a handful of times per frame.
static void set_vec3_idx(glid p, const char *base, int i, vec3 v) {
    char name[64];
    snprintf(name, sizeof name, "%s[%d]", base, i);
    gl_set_uniform_vec3(p, name, v.x, v.y, v.z);
}

static void set_float_idx(glid p, const char *base, int i, float v) {
    char name[64];
    snprintf(name, sizeof name, "%s[%d]", base, i);
    gl_set_uniform_float(p, name, v);
}

static void set_int_idx(glid p, const char *base, int i, int v) {
    char name[64];
    snprintf(name, sizeof name, "%s[%d]", base, i);
    gl_set_uniform_int(p, name, v);
}

int gbuffer_accum_upload_batch(gbuffer *g, const gbuffer_cull_result *c,
                               int start) {
    glid p = g->prog_accum;
    int n = c->count - start;
    if (n > GBUFFER_ACCUM_BATCH) n = GBUFFER_ACCUM_BATCH;
    if (n <= 0) return 0;

    for (int i = 0; i < n; i++) {
        const gbuffer_light *li = &c->lights[start + i];
        set_int_idx  (p, "u_light_kind",      i, (int)li->kind);
        set_vec3_idx (p, "u_light_pos",       i, li->pos);
        set_vec3_idx (p, "u_light_color",     i, li->color);
        set_float_idx(p, "u_light_radius",    i, li->radius);
        set_float_idx(p, "u_light_intensity", i, li->intensity);
    }
    gl_set_uniform_int(p, "u_light_count", n);
    return n;
}

void gbuffer_accumulate(gbuffer *g, const gbuffer_light_list *lights,
                        const frustum *fr, vec3 eye) {
    gbuffer_cull(lights, fr, eye, &g->culled);
    if (g->culled.dropped > 0)
        LOGD("gbuffer: dropped %d lights over cap", g->culled.dropped);

    // target the accumulation buffer, clear it, then additively splat
    glBindFramebuffer(GL_FRAMEBUFFER, g->accum_fbo);
    glViewport(0, 0, g->w, g->h);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);   // additive, each batch adds its light

    glUseProgram(g->prog_accum);
    // g-buffer attachments: units 0..2 color, 3 depth
    gbuffer_target_bind_textures(&g->target, 0);
    gl_set_uniform_int(g->prog_accum, "u_albedo",   0);
    gl_set_uniform_int(g->prog_accum, "u_normal",   1);
    gl_set_uniform_int(g->prog_accum, "u_material", 2);
    gl_set_uniform_int(g->prog_accum, "u_depth",    3);
    gl_set_uniform_mat4(g->prog_accum, "u_inv_proj", mat4_data(&g->inv_proj));
    gl_set_uniform_mat4(g->prog_accum, "u_view",     mat4_data(&g->view));
    gl_set_uniform_vec3(g->prog_accum, "u_eye", eye.x, eye.y, eye.z);

    glBindVertexArray(g->fs_vao);
    for (int start = 0; start < g->culled.count; start += GBUFFER_ACCUM_BATCH) {
        int n = gbuffer_accum_upload_batch(g, &g->culled, start);
        if (n <= 0) break;
        // fullscreen triangle, 3 verts generated in the vertex shader
        glDrawArrays(GL_TRIANGLES, 0, 3);
    }
    glBindVertexArray(0);

    glDisable(GL_BLEND);
    glDepthMask(GL_TRUE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
