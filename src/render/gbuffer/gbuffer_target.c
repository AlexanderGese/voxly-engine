#include "gbuffer_target.h"
#include "../../util/log.h"
#include <stddef.h>
// internal format per color attachment. index matches GBUFFER_MRT_COUNT.
static const struct {
    GLint  internal;
    GLenum fmt;
    GLenum type;
} k_specs[GBUFFER_MRT_COUNT] = {
    { GL_RGBA8,        GL_RGBA, GL_UNSIGNED_BYTE },        // albedo + ao
    { GL_RGB10_A2,     GL_RGBA, GL_UNSIGNED_INT_2_10_10_10_REV }, // normal
    { GL_RGBA8,        GL_RGBA, GL_UNSIGNED_BYTE },        // material
};
static void make_color_tex(glid *out, int i, int w, int h) {
    glGenTextures(1, out);
    glBindTexture(GL_TEXTURE_2D, *out);
    glTexImage2D(GL_TEXTURE_2D, 0, k_specs[i].internal, w, h, 0,
                 k_specs[i].fmt, k_specs[i].type, NULL);
    // point sample, the g-buffer is 1:1 with the screen so no filtering
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i,
                           GL_TEXTURE_2D, *out, 0);
}

int gbuffer_target_create(gbuffer_target *t, int w, int h) {
    t->w = w;
t->h = h;
glGenFramebuffers(1, &t->fbo);
glBindFramebuffer(GL_FRAMEBUFFER, t->fbo);
for (int i = 0;
i < GBUFFER_MRT_COUNT;
i++)
        make_color_tex(&t->tex[i], i, w, h);
// sampled depth, we reconstruct view position from this in the accum pass
glGenTextures(1, &t->depth_tex);
glBindTexture(GL_TEXTURE_2D, t->depth_tex);
glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, w, h, 0,
                 GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                           GL_TEXTURE_2D, t->depth_tex, 0);
GLenum bufs[GBUFFER_MRT_COUNT];
for (int i = 0;
i < GBUFFER_MRT_COUNT;
i++)
        bufs[i] = GL_COLOR_ATTACHMENT0 + i;
glDrawBuffers(GBUFFER_MRT_COUNT, bufs);
if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        LOGE("gbuffer: mrt framebuffer incomplete (%dx%d)", w, h);
        gbuffer_target_destroy(t);
        return 0;
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
return 1;
}

void gbuffer_target_destroy(gbuffer_target *t) {
    if (t->fbo) glDeleteFramebuffers(1, &t->fbo);
    for (int i = 0; i < GBUFFER_MRT_COUNT; i++)
        if (t->tex[i]) glDeleteTextures(1, &t->tex[i]);
    if (t->depth_tex) glDeleteTextures(1, &t->depth_tex);
    t->fbo = t->depth_tex = 0;
    for (int i = 0; i < GBUFFER_MRT_COUNT; i++) t->tex[i] = 0;
}

void gbuffer_target_bind(const gbuffer_target *t) {
    glBindFramebuffer(GL_FRAMEBUFFER, t->fbo);
glViewport(0, 0, t->w, t->h);
// we still set draw buffers here in case some other pass clobbered them
GLenum bufs[GBUFFER_MRT_COUNT];
for (int i = 0;
i < GBUFFER_MRT_COUNT;
i++)
        bufs[i] = GL_COLOR_ATTACHMENT0 + i;
glDrawBuffers(GBUFFER_MRT_COUNT, bufs);
}

void gbuffer_target_bind_textures(const gbuffer_target *t, int base) {
    for (int i = 0; i < GBUFFER_MRT_COUNT; i++) {
        glActiveTexture(GL_TEXTURE0 + base + i);
        glBindTexture(GL_TEXTURE_2D, t->tex[i]);
    }
    glActiveTexture(GL_TEXTURE0 + base + GBUFFER_MRT_COUNT);
    glBindTexture(GL_TEXTURE_2D, t->depth_tex);
}

void gbuffer_target_resize(gbuffer_target *t, int w, int h) {
    if (t->w == w && t->h == h) return;
gbuffer_target_destroy(t);
gbuffer_target_create(t, w, h);
}
