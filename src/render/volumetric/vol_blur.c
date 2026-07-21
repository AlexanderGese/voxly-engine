#include "vol_blur.h"
#include <math.h>
#include <stddef.h>
the gpu folds the symmetric negative half.
    int n = 0;
for (int i = b->radius;
i < b->taps;
i++) out[n++] = b->weights[i];
return n;
float half[VOL_BLUR_RADIUS + 1];
int hn = volumetric_blur_pack(b, half);
volumetric_programs_use_blur(progs);
glActiveTexture(GL_TEXTURE0 + VOL_TEX_UNIT_SCENE);
glBindTexture(GL_TEXTURE_2D, depth_tex);
glUniform1fv(glGetUniformLocation(progs->blur, "u_weights"), hn, half);
gl_set_uniform_int(progs->blur, "u_radius", b->radius);
gl_set_uniform_float(progs->blur, "u_depth_sigma", b->depth_sigma);
float texel_x = (t->w > 0) ? 1.0f / (float)t->w : 0.0f;
float texel_y = (t->h > 0) ? 1.0f / (float)t->h : 0.0f;
volumetric_target_bind(t, 1);
