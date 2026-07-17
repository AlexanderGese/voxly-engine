#include "ssao_gbuffer.h"

void ssaox_gbuffer_clear(ssaox_gbuffer *g) {
    g->tex_depth = 0;
    g->tex_normal = 0;
    g->tex_position = 0;
    g->proj = mat4_identity();
    g->inv_proj = mat4_identity();
    g->w = 0;
    g->h = 0;
}

int ssaox_gbuffer_valid(const ssaox_gbuffer *g) {
    if (!g->tex_depth)  return 0;
    if (!g->tex_normal) return 0;
    if (g->w <= 0 || g->h <= 0) return 0;
    return 1;
}

vec3 ssaox_gbuffer_view_pos(const ssaox_gbuffer *g, float u, float v, float depth) {
    // ndc in [-1,1]
    float nx = u * 2.0f - 1.0f;
    float ny = v * 2.0f - 1.0f;
    float nz = depth * 2.0f - 1.0f;

    // unproject: inv_proj * (ndc, 1), then divide by w. column major.
    const float (*m)[4] = g->inv_proj.m;
    float cx = m[0][0]*nx + m[1][0]*ny + m[2][0]*nz + m[3][0];
    float cy = m[0][1]*nx + m[1][1]*ny + m[2][1]*nz + m[3][1];
    float cz = m[0][2]*nx + m[1][2]*ny + m[2][2]*nz + m[3][2];
    float cw = m[0][3]*nx + m[1][3]*ny + m[2][3]*nz + m[3][3];

    vec3 p;
    if (cw == 0.0f) {
        // degenerate; just return the raw clip xyz and hope for the best
        p.x = cx; p.y = cy; p.z = cz;
        return p;
    }
    float inv = 1.0f / cw;
    p.x = cx * inv;
    p.y = cy * inv;
    p.z = cz * inv;
    return p;
}
