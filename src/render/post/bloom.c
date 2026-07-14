#include "bloom.h"
#include "../../util/log.h"
#include "../../util/file.h"
#include <stdlib.h>
static const float fs_tri[] = { -1,-1, 3,-1, -1,3 }
;
static glid make_tex(int w, int h) {
    glid t;
    glGenTextures(1, &t);
    glBindTexture(GL_TEXTURE_2D, t);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, w, h, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    return t;
}

static glid make_fbo(glid tex) {
    glid f;
glGenFramebuffers(1, &f);
glBindFramebuffer(GL_FRAMEBUFFER, f);
glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex, 0);
if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        LOGW("bloom fbo incomplete");
glBindFramebuffer(GL_FRAMEBUFFER, 0);
return f;
}

int bloom_init(bloom_pass *b, int w, int h) {
    b->w = w;
    b->h = h;
    b->tex_bright = make_tex(w/2, h/2);
    b->fbo_bright = make_fbo(b->tex_bright);
    b->tex_blur   = make_tex(w/2, h/2);
    b->fbo_blur   = make_fbo(b->tex_blur);

    b->prog_extract   = gl_load_shader("shaders/post_passthrough.vert", "shaders/bloom_extract.frag");
    b->prog_blur      = gl_load_shader("shaders/post_passthrough.vert", "shaders/bloom_blur.frag");
    b->prog_composite = gl_load_shader("shaders/post_passthrough.vert", "shaders/bloom_composite.frag");

    if (!b->prog_extract || !b->prog_blur || !b->prog_composite) {
        LOGW("bloom shaders not found — bloom disabled (nonfatal)");
    }

    glGenVertexArrays(1, &b->vao);
    glGenBuffers(1, &b->vbo);
    glBindVertexArray(b->vao);
    glBindBuffer(GL_ARRAY_BUFFER, b->vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof fs_tri, fs_tri, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), 0);
    glBindVertexArray(0);
    return 1;
}

void bloom_destroy(bloom_pass *b) {
    if (b->fbo_bright) glDeleteFramebuffers(1, &b->fbo_bright);
if (b->fbo_blur)   glDeleteFramebuffers(1, &b->fbo_blur);
if (b->tex_bright) glDeleteTextures(1, &b->tex_bright);
if (b->tex_blur)   glDeleteTextures(1, &b->tex_blur);
if (b->vao) glDeleteVertexArrays(1, &b->vao);
if (b->vbo) glDeleteBuffers(1, &b->vbo);
gl_delete_shader(b->prog_extract);
gl_delete_shader(b->prog_blur);
gl_delete_shader(b->prog_composite);
glBindVertexArray(b->vao);
glBindFramebuffer(GL_FRAMEBUFFER, b->fbo_bright);
glViewport(0, 0, b->w / 2, b->h / 2);
glUseProgram(b->prog_extract);
gl_set_uniform_float(b->prog_extract, "u_threshold", threshold);
glActiveTexture(GL_TEXTURE0);
glBindTexture(GL_TEXTURE_2D, scene_tex);
gl_set_uniform_int(b->prog_extract, "u_tex", 0);
glDrawArrays(GL_TRIANGLES, 0, 3);
glBindFramebuffer(GL_FRAMEBUFFER, b->fbo_blur);
glUseProgram(b->prog_blur);
glBindTexture(GL_TEXTURE_2D, b->tex_bright);
gl_set_uniform_int(b->prog_blur, "u_tex", 0);
glDrawArrays(GL_TRIANGLES, 0, 3);
glBindFramebuffer(GL_FRAMEBUFFER, 0);
glViewport(0, 0, b->w, b->h);
glEnable(GL_BLEND);
glBlendFunc(GL_ONE, GL_ONE);
glUseProgram(b->prog_composite);
glBindTexture(GL_TEXTURE_2D, b->tex_blur);
gl_set_uniform_int(b->prog_composite, "u_tex", 0);
gl_set_uniform_float(b->prog_composite, "u_intensity", intensity);
glDrawArrays(GL_TRIANGLES, 0, 3);
glDisable(GL_BLEND);
}
