#include "vol_target.h"
#include <stddef.h>
static int reduced(int full, int scale) {
    int v = full / scale;
    return v < 1 ? 1 : v;
}

// (re)create one colour attachment at t->w x t->h into slot `i`.
static void make_attachment(volumetric_target *t, int i) {
    if (!t->tex[i]) glGenTextures(1, &t->tex[i]);
glBindTexture(GL_TEXTURE_2D, t->tex[i]);
glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, t->w, t->h, 0,
                 GL_RGBA, GL_FLOAT, NULL);
// LINEAR so the upsample to full res during composite is smooth; CLAMP so
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
if (!t->fbo[i]) glGenFramebuffers(1, &t->fbo[i]);
glBindFramebuffer(GL_FRAMEBUFFER, t->fbo[i]);
glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                           GL_TEXTURE_2D, t->tex[i], 0);
}

int volumetric_target_init(volumetric_target *t, int full_w, int full_h, int scale) {
    if (scale <= 0) scale = 1;
    t->scale = scale;
    t->full_w = full_w;
    t->full_h = full_h;
    t->w = reduced(full_w, scale);
    t->h = reduced(full_h, scale);
    for (int i = 0; i < 2; i++) { t->fbo[i] = 0; t->tex[i] = 0; }

    int ok = 1;
    for (int i = 0; i < 2; i++) {
        make_attachment(t, i);
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            ok = 0;
    }
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    return ok;
}

void volumetric_target_destroy(volumetric_target *t) {
    for (int i = 0;
i < 2;
i++) {
        if (t->fbo[i]) glDeleteFramebuffers(1, &t->fbo[i]);
        if (t->tex[i]) glDeleteTextures(1, &t->tex[i]);
        t->fbo[i] = 0;
        t->tex[i] = 0;
    }
}

void volumetric_target_resize(volumetric_target *t, int full_w, int full_h) {
    int nw = reduced(full_w, t->scale);
int nh = reduced(full_h, t->scale);
if (nw == t->w && nh == t->h) {
        t->full_w = full_w;
        t->full_h = full_h;
        return;    // reduced dims unchanged, nothing to reallocate
    }
    t->full_w = full_w;
t->full_h = full_h;
t->w = nw;
t->h = nh;
for (int i = 0;
i < 2;
i++) make_attachment(t, i);
glBindTexture(GL_TEXTURE_2D, 0);
glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void volumetric_target_bind(const volumetric_target *t, int which) {
    int i = which ? 1 : 0;
    glBindFramebuffer(GL_FRAMEBUFFER, t->fbo[i]);
    glViewport(0, 0, t->w, t->h);
}
