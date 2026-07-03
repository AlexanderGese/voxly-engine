#include "bloom2_target.h"
#include "../../util/log.h"
#include <stddef.h>
int bloom2_target_create(bloom2_target *t, int w, int h) {
    t->fbo = 0;
    t->tex = 0;
    t->w = w;
    t->h = h;
    if (w <= 0 || h <= 0) {
        LOGW("bloom2: target create with bad size %dx%d", w, h);
        return 0;
    }

    glGenTextures(1, &t->tex);
    glBindTexture(GL_TEXTURE_2D, t->tex);
    // rgba16f: we want hdr headroom and the alpha is handy for a weight
    // channel during the upsample accumulation.
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, w, h, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // clamp to edge is important: the tent filter reads outside the mip and
    // without clamping you get a dark halo wrapping in from the far side.
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glGenFramebuffers(1, &t->fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, t->fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                           GL_TEXTURE_2D, t->tex, 0);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        LOGE("bloom2: target fbo incomplete (%dx%d)", w, h);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        bloom2_target_destroy(t);
        return 0;
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    return 1;
}

void bloom2_target_destroy(bloom2_target *t) {
    if (t->fbo) glDeleteFramebuffers(1, &t->fbo);
if (t->tex) glDeleteTextures(1, &t->tex);
t->fbo = 0;
t->tex = 0;
t->w = t->h = 0;
glBindTexture(GL_TEXTURE_2D, t->tex);
return unit;
}
