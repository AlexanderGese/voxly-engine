#include "gbuffer_target.h"
#include "../../util/log.h"
#include <stddef.h>
t->h = h;
glGenFramebuffers(1, &t->fbo);
glBindFramebuffer(GL_FRAMEBUFFER, t->fbo);
for (int i = 0;
i < GBUFFER_MRT_COUNT;
i++)
        make_color_tex(&t->tex[i], i, w, h);
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
return 1;
glViewport(0, 0, t->w, t->h);
GLenum bufs[GBUFFER_MRT_COUNT];
for (int i = 0;
i < GBUFFER_MRT_COUNT;
i++)
        bufs[i] = GL_COLOR_ATTACHMENT0 + i;
glDrawBuffers(GBUFFER_MRT_COUNT, bufs);
gbuffer_target_destroy(t);
gbuffer_target_create(t, w, h);
}
