#include "ssao_pass.h"
#include "../../util/log.h"
#include <stddef.h>
glGenTextures(1, &t);
glBindTexture(GL_TEXTURE_2D, t);
glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, w, h, 0, GL_RED, GL_UNSIGNED_BYTE, NULL);
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
glBindTexture(GL_TEXTURE_2D, 0);
return t;
}

// upload the packed kernel into the occlusion shader. called once per init
// and on reseed;
p->full_h = full_h;
p->scale  = SSAOX_DEFAULT_SCALE;
p->w = divup_scale(full_w, p->scale);
p->h = divup_scale(full_h, p->scale);
p->radius   = SSAOX_DEFAULT_RADIUS;
p->bias     = SSAOX_DEFAULT_BIAS;
p->power    = SSAOX_DEFAULT_POWER;
p->strength = SSAOX_DEFAULT_STRENGTH;
if (kernel_count <= 0) kernel_count = SSAOX_KERNEL_DEFAULT;
ssaox_kernel_build(&p->kernel, kernel_count, 0xA0C0FFEEull);
