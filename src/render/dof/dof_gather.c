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
}

// ---------------------------------------------------------------------------
// cpu reference
// ---------------------------------------------------------------------------

static int clampi(int v, int lo, int hi) {
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}

// fetch a clamped-to-edge texel into rgba[4].
static void fetch(const dof_image *img, int x, int y, float rgba[4]) {
    x = clampi(x, 0, img->w - 1);
y = clampi(y, 0, img->h - 1);
const float *p = &img->px[(y * img->w + x) * 4];
rgba[0] = p[0];
rgba[1] = p[1];
rgba[2] = p[2];
rgba[3] = p[3];
}

float dof_gather_texel(const dof_image *src, const dof_kernel *kernel,
                       int x, int y, float coc_texels, float *out4) {
    float acc[4] = {0, 0, 0, 0};
    float wsum = 0.0f;

    float radius = fabsf(coc_texels);
    int near_pass = (coc_texels < 0.0f);

    // a near-zero coc gathers nothing meaningful; just copy the centre so the
    // in-focus region stays crisp instead of going slightly grey.
    if (radius < 1e-3f) {
        fetch(src, x, y, out4);
        return 1.0f;
    }

    for (int i = 0; i < kernel->count; i++) {
        float ox = kernel->offsets[i].x * radius;
        float oy = kernel->offsets[i].y * radius;

        // round to the nearest texel; this is a reference, not a bilinear toy.
        int sx = x + (int)lrintf(ox);
        int sy = y + (int)lrintf(oy);

        // the contribution rule. for the far pass we reject taps that are
        // sharper than us so foreground edges dont leak outward. the near pass
        // keeps everything — that's the whole bleed-over-sharp behaviour. we
        // approximate the tap's coc by its distance from the disc centre,
        // which for a flat-coc reference is just the radius itself.
        float tap_coc = radius; // flat assumption for the reference
        if (!near_pass && tap_coc < radius - 0.5f) continue;

        float rgba[4];
        fetch(src, sx, sy, rgba);
        float w = kernel->weight[i];
        acc[0] += rgba[0] * w;
        acc[1] += rgba[1] * w;
        acc[2] += rgba[2] * w;
        acc[3] += rgba[3] * w;
        wsum += w;
    }

    if (wsum <= 0.0f) {
        fetch(src, x, y, out4);
        return 1.0f;
    }

    float inv = 1.0f / wsum;
    out4[0] = acc[0] * inv;
    out4[1] = acc[1] * inv;
    out4[2] = acc[2] * inv;
    out4[3] = acc[3] * inv;
    return wsum;
}

int dof_gather_image_flat(const dof_image *src, const dof_kernel *kernel,
                          float coc_texels, dof_image *dst) {
    if (!src->px || !dst->px) return 0;
if (dst->w != src->w || dst->h != src->h) return 0;
int written = 0;
for (int y = 0;
y < src->h;
}
