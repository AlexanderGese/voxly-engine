#include "ssr_gbuffer.h"

#include <math.h>

// view-space reconstruction + reprojection. these two functions are the
// contract between the cpu march and the glsl: if they disagree the cpu tests
// stop being a meaningful reference. so keep them in lockstep.

void ssrx_gbuffer_clear(ssrx_gbuffer *g) {
    g->tex_depth = g->tex_normal = g->tex_color = g->tex_material = 0;
    g->proj     = mat4_identity();
    g->inv_proj = mat4_identity();
    g->w = g->h = 0;
}

int ssrx_gbuffer_valid(const ssrx_gbuffer *g) {
    // material is optional; the other three are not.
    if (!g->tex_depth || !g->tex_normal || !g->tex_color) return 0;
    if (g->w <= 0 || g->h <= 0) return 0;
    return 1;
}

// multiply inv_proj by a clip-space vec4 by hand. mat4 only ships a vec3
// (w=1) multiply and we genuinely need the w divide here, so unroll it.
static void inv_proj_mul(const mat4 *m, float cx, float cy, float cz, float cw,
                         float out[4]) {
    const float (*c)[4] = m->m; // column major: c[col][row]
    out[0] = c[0][0]*cx + c[1][0]*cy + c[2][0]*cz + c[3][0]*cw;
    out[1] = c[0][1]*cx + c[1][1]*cy + c[2][1]*cz + c[3][1]*cw;
    out[2] = c[0][2]*cx + c[1][2]*cy + c[2][2]*cz + c[3][2]*cw;
    out[3] = c[0][3]*cx + c[1][3]*cy + c[2][3]*cz + c[3][3]*cw;
}

vec3 ssrx_gbuffer_view_pos(const ssrx_gbuffer *g, float u, float v, float depth) {
    // uv [0,1] + ndc depth [0,1] -> ndc clip [-1,1] -> view via inv_proj.
    float nx = u * 2.0f - 1.0f;
    float ny = v * 2.0f - 1.0f;
    float nz = depth * 2.0f - 1.0f;

    float view[4];
    inv_proj_mul(&g->inv_proj, nx, ny, nz, 1.0f, view);

    // perspective divide. degenerate w means the point is on/behind the eye;
    // hand back something harmless rather than an inf.
    if (fabsf(view[3]) < 1e-8f) return vec3_new(0.0f, 0.0f, 0.0f);
    float inv_w = 1.0f / view[3];
    return vec3_new(view[0] * inv_w, view[1] * inv_w, view[2] * inv_w);
}

// forward project a view-space point through g->proj.
static void proj_mul(const mat4 *m, vec3 p, float out[4]) {
    const float (*c)[4] = m->m;
    out[0] = c[0][0]*p.x + c[1][0]*p.y + c[2][0]*p.z + c[3][0];
    out[1] = c[0][1]*p.x + c[1][1]*p.y + c[2][1]*p.z + c[3][1];
    out[2] = c[0][2]*p.x + c[1][2]*p.y + c[2][2]*p.z + c[3][2];
    out[3] = c[0][3]*p.x + c[1][3]*p.y + c[2][3]*p.z + c[3][3];
}

int ssrx_gbuffer_project(const ssrx_gbuffer *g, vec3 view_pos,
                         vec2 *out_uv, float *out_depth) {
    float clip[4];
    proj_mul(&g->proj, view_pos, clip);

    // w<=0 is at/behind the near plane — not on screen, bail untouched.
    if (clip[3] <= 1e-8f) return 0;

    float inv_w = 1.0f / clip[3];
    float nx = clip[0] * inv_w;
    float ny = clip[1] * inv_w;
    float nz = clip[2] * inv_w;

    if (out_uv) {
        out_uv->x = nx * 0.5f + 0.5f;
        out_uv->y = ny * 0.5f + 0.5f;
    }
    if (out_depth) *out_depth = nz * 0.5f + 0.5f;
    return 1;
}
