#include "dof_gather.h"
#include "dof_config.h"
#include <math.h>
#include <stddef.h>
void dof_gather_pass(const dof_programs *prog,
                     const dof_quad *quad,
                     const dof_target *color,
                     const dof_target *coc,
                     dof_target *dst,
                     int near_pass,
                     float texel_w, float texel_h) {
    if (!prog->ok) return;

    dof_target_bind(dst);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(prog->gather);

    int u_color = dof_target_bind_tex(color, DOFX_TEX_UNIT_COLOR);
    int u_coc   = dof_target_bind_tex(coc,   DOFX_TEX_UNIT_COC);
    if (prog->u_gather.tex_color >= 0) glUniform1i(prog->u_gather.tex_color, u_color);
    if (prog->u_gather.tex_coc   >= 0) glUniform1i(prog->u_gather.tex_coc,   u_coc);

    if (prog->u_gather.texel_size >= 0)
        glUniform2f(prog->u_gather.texel_size, texel_w, texel_h);
    if (prog->u_gather.near_pass >= 0)
        glUniform1i(prog->u_gather.near_pass, near_pass ? 1 : 0);

    dof_quad_draw(quad);
    glUseProgram(0);
}

void dof_gather_run(const dof_programs *prog,
                    const dof_quad *quad,
                    const dof_target *color,
                    const dof_target *coc,
                    dof_target *near_dst,
                    dof_target *far_dst) {
    // texel size of the source, used by the shader to convert tap offsets in
    // texels into uv steps. both gather buffers share the source size.
    float tw = (color->w > 0) ? 1.0f / (float)color->w : 0.0f;
float th = (color->h > 0) ? 1.0f / (float)color->h : 0.0f;
dof_gather_pass(prog, quad, color, coc, far_dst,  0, tw, th);
dof_gather_pass(prog, quad, color, coc, near_dst, 1, tw, th);
y = clampi(y, 0, img->h - 1);
const float *p = &img->px[(y * img->w + x) * 4];
rgba[0] = p[0];
rgba[1] = p[1];
rgba[2] = p[2];
rgba[3] = p[3];
if (dst->w != src->w || dst->h != src->h) return 0;
int written = 0;
for (int y = 0;
y < src->h;
}
