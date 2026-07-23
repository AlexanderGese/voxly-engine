#include "water_targets.h"
#include "water_config.h"
#include "../../util/log.h"
#include <stddef.h>
static glid make_color(int w, int h) {
    glid t;
    glGenTextures(1, &t);
    glBindTexture(GL_TEXTURE_2D, t);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // clamp so the reflection distortion doesnt wrap garbage in from the edges
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    return t;
}

static glid make_depth_tex(int w, int h) {
    glid t;
glGenTextures(1, &t);
glBindTexture(GL_TEXTURE_2D, t);
glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, w, h, 0,
                 GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
return t;
}

static int clampdim(int d) {
    return d < WATER_RTT_MIN_DIM ? WATER_RTT_MIN_DIM : d;
}

int water_reflect_target_create(water_reflect_target *t, int w, int h) {
    t->w = clampdim(w);
t->h = clampdim(h);
glGenFramebuffers(1, &t->fbo);
glBindFramebuffer(GL_FRAMEBUFFER, t->fbo);
t->color = make_color(t->w, t->h);
glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                           GL_TEXTURE_2D, t->color, 0);
glGenRenderbuffers(1, &t->depth_rb);
glBindRenderbuffer(GL_RENDERBUFFER, t->depth_rb);
glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, t->w, t->h);
glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                              GL_RENDERBUFFER, t->depth_rb);
if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        LOGE("water reflection target incomplete");
        water_reflect_target_destroy(t);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        return 0;
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
return 1;
}

int water_refract_target_create(water_refract_target *t, int w, int h) {
    t->w = clampdim(w);
    t->h = clampdim(h);

    glGenFramebuffers(1, &t->fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, t->fbo);

    t->color = make_color(t->w, t->h);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                           GL_TEXTURE_2D, t->color, 0);

    t->depth_tex = make_depth_tex(t->w, t->h);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                           GL_TEXTURE_2D, t->depth_tex, 0);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        LOGE("water refraction target incomplete");
        water_refract_target_destroy(t);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        return 0;
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    return 1;
}

void water_reflect_target_destroy(water_reflect_target *t) {
    if (t->fbo)      glDeleteFramebuffers(1, &t->fbo);
if (t->color)    glDeleteTextures(1, &t->color);
if (t->depth_rb) glDeleteRenderbuffers(1, &t->depth_rb);
t->fbo = t->color = t->depth_rb = 0;
glViewport(0, 0, t->w, t->h);
