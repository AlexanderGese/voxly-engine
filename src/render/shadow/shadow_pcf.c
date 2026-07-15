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
i < k->count;
i++) k->taps[i].weight /= (float)k->count;
if (cascade >= csm->count) cascade = csm->count - 1;
float maxr = 0.0f;
for (int i = 0;
i < k->count;
for (int i = 0;
i < csm->count;
for (int i = 0;
i < csm->count;
i++) far_planes[i] = csm->splits[i + 1];
GLint loc = glGetUniformLocation(prog, "u_shadow_splits");
if (loc >= 0) glUniform1fv(loc, csm->count, far_planes);
}
