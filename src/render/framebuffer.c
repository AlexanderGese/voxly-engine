#include "framebuffer.h"
#include "../util/log.h"

#include <stddef.h>

int framebuffer_create(framebuffer *fb, int w, int h) {
    fb->w = w;
    fb->h = h;

    glGenFramebuffers(1, &fb->fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fb->fbo);

    glGenTextures(1, &fb->color);
    glBindTexture(GL_TEXTURE_2D, fb->color);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                           GL_TEXTURE_2D, fb->color, 0);

    glGenRenderbuffers(1, &fb->depth);
    glBindRenderbuffer(GL_RENDERBUFFER, fb->depth);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, w, h);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                              GL_RENDERBUFFER, fb->depth);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        LOGE("framebuffer incomplete");
        framebuffer_destroy(fb);
        return 0;
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    return 1;
}

void framebuffer_destroy(framebuffer *fb) {
    if (fb->fbo)   glDeleteFramebuffers(1, &fb->fbo);
    if (fb->color) glDeleteTextures(1, &fb->color);
    if (fb->depth) glDeleteRenderbuffers(1, &fb->depth);
    fb->fbo = fb->color = fb->depth = 0;
}

void framebuffer_bind(const framebuffer *fb) {
    glBindFramebuffer(GL_FRAMEBUFFER, fb->fbo);
    glViewport(0, 0, fb->w, fb->h);
}

void framebuffer_unbind(void) {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void framebuffer_resize(framebuffer *fb, int w, int h) {
    if (fb->w == w && fb->h == h) return;
    framebuffer_destroy(fb);
    framebuffer_create(fb, w, h);
}
