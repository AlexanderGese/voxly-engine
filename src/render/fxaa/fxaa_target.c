#include "fxaa_target.h"
#include "../../util/log.h"

#include <stddef.h>

static glid make_tex(int w, int h) {
    glid t;
    glGenTextures(1, &t);
    glBindTexture(GL_TEXTURE_2D, t);
    // rgba8: ldr scene in rgb, luma in alpha. fxaa is a post-tonemap pass so
    // we never need hdr headroom here.
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, NULL);
    // linear is not optional: the main pass takes bilinear taps at half-texel
    // offsets to blend across edges. nearest would defeat the whole thing.
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_2D, 0);
    return t;
}

int fxaa_target_create(fxaa_target *t, int w, int h) {
    t->fbo = 0;
    t->tex = 0;
    t->w = w;
    t->h = h;
    if (w <= 0 || h <= 0) {
        LOGW("fxaa: target create with bad size %dx%d", w, h);
        return 0;
    }

    t->tex = make_tex(w, h);
    glGenFramebuffers(1, &t->fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, t->fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                           GL_TEXTURE_2D, t->tex, 0);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        LOGE("fxaa: target fbo incomplete (%dx%d)", w, h);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        fxaa_target_destroy(t);
        return 0;
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    return 1;
}

void fxaa_target_destroy(fxaa_target *t) {
    if (t->fbo) glDeleteFramebuffers(1, &t->fbo);
    if (t->tex) glDeleteTextures(1, &t->tex);
    t->fbo = 0;
    t->tex = 0;
    t->w = t->h = 0;
}

int fxaa_target_resize(fxaa_target *t, int w, int h) {
    if (w <= 0 || h <= 0) return 0;
    if (t->w == w && t->h == h && t->tex) return 1;

    // keep the fbo, just swap the attachment. cheaper than a full teardown and
    // it keeps the fbo id stable for anyone holding it.
    if (t->tex) glDeleteTextures(1, &t->tex);
    t->tex = make_tex(w, h);
    t->w = w;
    t->h = h;

    if (!t->fbo) glGenFramebuffers(1, &t->fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, t->fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                           GL_TEXTURE_2D, t->tex, 0);
    int ok = (glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    if (!ok) LOGE("fxaa: target resize fbo incomplete (%dx%d)", w, h);
    return ok;
}

void fxaa_target_bind(const fxaa_target *t) {
    glBindFramebuffer(GL_FRAMEBUFFER, t->fbo);
    glViewport(0, 0, t->w, t->h);
}

int fxaa_target_bind_tex(const fxaa_target *t, int unit) {
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(GL_TEXTURE_2D, t->tex);
    return unit;
}
