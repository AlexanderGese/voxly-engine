#include "fxaa_target.h"
#include "../../util/log.h"
#include <stddef.h>
t->tex = 0;
t->w = w;
t->h = h;
glGenFramebuffers(1, &t->fbo);
glBindFramebuffer(GL_FRAMEBUFFER, t->fbo);
glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                           GL_TEXTURE_2D, t->tex, 0);
return 1;
if (t->w == w && t->h == h && t->tex) return 1;
if (t->tex) glDeleteTextures(1, &t->tex);
