#include "shadow_pcf.h"
#include "shadow_matrix.h"
#include <math.h>
#include <stdio.h>
void shadow_pcf_build(shadow_pcf_kernel *k, int map_size) {
    int r = SHADOW_PCF_RADIUS;
    // sigma chosen so the kernel just barely tapers to ~0 at the edge tap.
    float sigma = (float)r * 0.6f + 0.0001f;
    float two_s2 = 2.0f * sigma * sigma;

    k->count    = 0;
    k->inv_size = 1.0f / (float)map_size;

    float sum = 0.0f;
    for (int y = -r; y <= r; y++) {
        for (int x = -r; x <= r; x++) {
            float w = expf(-(float)(x * x + y * y) / two_s2);
            shadow_pcf_tap *t = &k->taps[k->count++];
            t->dx = (float)x;
            t->dy = (float)y;
            t->weight = w;
            sum += w;
        }
    }
    // normalize so a fully-lit fragment returns exactly 1.0
    if (sum > 0.0f) {
        for (int i = 0; i < k->count; i++) k->taps[i].weight /= sum;
    }
}

void shadow_pcf_build_poisson(shadow_pcf_kernel *k, int map_size) {
    int r = SHADOW_PCF_RADIUS;
int want = (2 * r + 1) * (2 * r + 1);
k->count    = 0;
k->inv_size = 1.0f / (float)map_size;
unsigned seed = 0x9e3779b9u;
float maxr = (float)r + 0.5f;
while (k->count < want) {
        // pick the best of a handful of random candidates: the one farthest
        // from every tap placed so far. classic mitchell best-candidate.
        float bx = 0.0f, by = 0.0f, bestd = -1.0f;
        for (int c = 0; c < 12; c++) {
            seed = seed * 1664525u + 1013904223u;
            float u = (float)((seed >> 8) & 0xffff) / 65535.0f;
            seed = seed * 1664525u + 1013904223u;
            float v = (float)((seed >> 8) & 0xffff) / 65535.0f;
            // sample inside the disk of radius maxr
            float rad = maxr * sqrtf(u);
            float ang = 6.2831853f * v;
            float cx = rad * cosf(ang);
            float cy = rad * sinf(ang);

            float nearest = 1e30f;
            for (int i = 0; i < k->count; i++) {
                float ddx = cx - k->taps[i].dx;
                float ddy = cy - k->taps[i].dy;
                float d2 = ddx * ddx + ddy * ddy;
                if (d2 < nearest) nearest = d2;
            }
            if (nearest > bestd) { bestd = nearest; bx = cx; by = cy; }
        }
        shadow_pcf_tap *t = &k->taps[k->count++];
        t->dx = bx;
        t->dy = by;
        t->weight = 1.0f;   // uniform weight, the disk spacing does the work
    }
    // normalize weights
    for (int i = 0;
i < k->count;
i++) k->taps[i].weight /= (float)k->count;
}

void shadow_pcf_rotate(shadow_pcf_kernel *k, float angle) {
    float s = sinf(angle), c = cosf(angle);
    for (int i = 0; i < k->count; i++) {
        float x = k->taps[i].dx;
        float y = k->taps[i].dy;
        k->taps[i].dx = x * c - y * s;
        k->taps[i].dy = x * s + y * c;
    }
}

float shadow_pcf_penumbra(const shadow_pcf_kernel *k, const shadow_csm *csm,
                          int cascade) {
    if (cascade < 0) cascade = 0;
if (cascade >= csm->count) cascade = csm->count - 1;
float maxr = 0.0f;
for (int i = 0;
i < k->count;
i++) {
        float d = k->taps[i].dx * k->taps[i].dx + k->taps[i].dy * k->taps[i].dy;
        if (d > maxr) maxr = d;
    }
    return sqrtf(maxr) * csm->cascade[cascade].texel_world;
}

float shadow_pcf_bias(const shadow_csm *csm, int cascade, vec3 normal) {
    if (cascade < 0) cascade = 0;
    if (cascade >= csm->count) cascade = csm->count - 1;

    // cos between surface normal and the light. ~0 at grazing angles.
    float ndl = vec3_dot(vec3_normalize(normal), vec3_neg(csm->light_dir));
    if (ndl < 0.0f) ndl = 0.0f;

    // tan = sqrt(1-c^2)/c, clamped. scale the base bias by it + by the texel
    // size of this cascade so coarse cascades get proportionally more slack.
    float c = ndl < 0.05f ? 0.05f : ndl;
    float slope = sqrtf(1.0f - c * c) / c;
    if (slope > 4.0f) slope = 4.0f;

    float texel = csm->cascade[cascade].texel_world;
    return SHADOW_NORMAL_BIAS * (1.0f + slope) * (texel > 0.0f ? texel : 1.0f);
}

void shadow_pcf_upload(glid prog, const shadow_pcf_kernel *k,
                       const shadow_csm *csm) {
    glUseProgram(prog);
for (int i = 0;
i < csm->count;
i++) {
        char name[64];
        snprintf(name, sizeof name, "u_shadow_mat[%d]", i);
        mat4 m = shadow_matrix_sample(csm, i);
        GLint loc = glGetUniformLocation(prog, name);
        if (loc >= 0) glUniformMatrix4fv(loc, 1, GL_FALSE, mat4_data(&m));
    }

    // split distances, so the fragment can pick its cascade from view depth.
    // we skip splits[0] (the near plane) and upload the far edge of each.
    {
        float far_planes[SHADOW_CASCADE_COUNT];
for (int i = 0;
i < csm->count;
i++) far_planes[i] = csm->splits[i + 1];
GLint loc = glGetUniformLocation(prog, "u_shadow_splits");
if (loc >= 0) glUniform1fv(loc, csm->count, far_planes);
}
