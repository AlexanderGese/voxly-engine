#include "dof_target.h"
#include "../../util/log.h"

#include <stddef.h>

int dof_target_create(dof_target *t, int w, int h) {
    t->fbo = 0;
    t->tex = 0;
    t->w = w;
    t->h = h;
    if (w <= 0 || h <= 0) {
        LOGW("dof: target create with bad size %dx%d", w, h);
        return 0;
    }

    glGenTextures(1, &t->tex);
    glBindTexture(GL_TEXTURE_2D, t->tex);
    // rgba16f: keep highlight energy so bokeh discs stay bright. alpha carries
    // the accumulated coverage weight out of the gather so the composite can
    // normalize without a second pass.
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, w, h, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // clamp: the upsample reads slightly outside the half-res buffer and we
    // dont want the far edge wrapping into the near edge.
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glGenFramebuffers(1, &t->fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, t->fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                           GL_TEXTURE_2D, t->tex, 0);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        LOGE("dof: target fbo incomplete (%dx%d)", w, h);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        dof_target_destroy(t);
        return 0;
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    return 1;
}

void dof_target_destroy(dof_target *t) {
    if (t->fbo) glDeleteFramebuffers(1, &t->fbo);
    if (t->tex) glDeleteTextures(1, &t->tex);
    t->fbo = 0;
    t->tex = 0;
    t->w = t->h = 0;
}

void dof_target_bind(const dof_target *t) {
    glBindFramebuffer(GL_FRAMEBUFFER, t->fbo);
    glViewport(0, 0, t->w, t->h);
}

int dof_target_bind_tex(const dof_target *t, int unit) {
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(GL_TEXTURE_2D, t->tex);
    return unit;
}

int dof_target_resize(dof_target *t, int w, int h) {
    if (w == t->w && h == t->h && t->fbo) return 1; // nothing to do
    dof_target_destroy(t);
    return dof_target_create(t, w, h);
}
