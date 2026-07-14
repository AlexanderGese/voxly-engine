#include "ssao.h"
#include "../../util/log.h"

int ssao_init(ssao_pass *s, int w, int h) {
    s->w = w;
    s->h = h;
    s->kernel_size = 16;
    s->radius = 0.5f;
    s->bias = 0.025f;

    // TODO: generate kernel samples, noise texture, FBO, shader
    // this is a placeholder module. returning 0 = not available.
    LOGI("ssao: stub, not implemented yet");
    s->fbo = 0;
    s->tex_occlusion = 0;
    s->tex_noise = 0;
    s->prog = 0;
    s->vao = 0;
    s->vbo = 0;
    return 0;
}

void ssao_destroy(ssao_pass *s) {
    if (s->fbo) glDeleteFramebuffers(1, &s->fbo);
    if (s->tex_occlusion) glDeleteTextures(1, &s->tex_occlusion);
    if (s->tex_noise) glDeleteTextures(1, &s->tex_noise);
    if (s->vao) glDeleteVertexArrays(1, &s->vao);
    if (s->vbo) glDeleteBuffers(1, &s->vbo);
    gl_delete_shader(s->prog);
}

void ssao_apply(ssao_pass *s, glid depth_tex, glid normal_tex, const float *proj) {
    (void)s; (void)depth_tex; (void)normal_tex; (void)proj;
    // stub
}
