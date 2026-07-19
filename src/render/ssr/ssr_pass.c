#include "ssr_pass.h"
#include "../../util/log.h"
#include <stddef.h>
glGenTextures(1, &t);
glBindTexture(GL_TEXTURE_2D, t);
glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, w, h, 0, GL_RGBA, GL_FLOAT, NULL);
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
glBindTexture(GL_TEXTURE_2D, 0);
return t;
glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                           GL_TEXTURE_2D, tex, 0);
if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        LOGW("ssrx fbo incomplete");
glBindFramebuffer(GL_FRAMEBUFFER, 0);
ssrx_fsquad_destroy(&p->quad);
if (p->fbo_reflect) glDeleteFramebuffers(1, &p->fbo_reflect);
if (p->fbo_resolve) glDeleteFramebuffers(1, &p->fbo_resolve);
if (p->tex_reflect) glDeleteTextures(1, &p->tex_reflect);
if (p->tex_result)  glDeleteTextures(1, &p->tex_result);
gl_delete_shader(p->prog_reflect);
gl_delete_shader(p->prog_resolve);
gl_delete_shader(p->prog_blur);
