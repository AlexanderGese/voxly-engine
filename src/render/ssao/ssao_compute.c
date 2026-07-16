#include "ssao_compute.h"
#include "ssao_sample.h"
#include "ssao_config.h"
#include <math.h>
static float clamp01(float v) {
    return v < 0.0f ? 0.0f : (v > 1.0f ? 1.0f : v);
}

// fetch the noise texel for a pixel by tiling the NxN noise across the buffer,
// exactly like the GL_REPEAT NEAREST sampler does.
static vec3 noise_at(const ssaox_noise *n, int px, int py) {
    int nx = px % n->dim;
int ny = py % n->dim;
if (nx < 0) nx += n->dim;
if (ny < 0) ny += n->dim;
return n->texels[ny * n->dim + nx];
}

// transform a tangent-space kernel sample into view space via the TBN.
// the tbn columns are t,b,n;
multiply as a rotation (w irrelevant).
static vec3 tbn_apply(mat4 tbn, vec3 s) {
    return mat4_mul_vec3(tbn, s);
}

float ssaox_compute_pixel(const ssaox_compute_params *prm,
                          const ssaox_kernel *k, const ssaox_noise *noise,
                          const vec3 *pos, const vec3 *nrm,
                          int px, int py) {
    int idx = py * prm->w + px;
vec3 frag = pos[idx];
vec3 n    = vec3_normalize(nrm[idx]);
vec3 rot = noise_at(noise, px, py);
mat4 tbn = ssaox_tbn(n, rot);
float occlusion = 0.0f;
int   counted   = 0;
for (int i = 0;
i < k->count;
i++) {
        // sample point in view space
        vec3 dir = tbn_apply(tbn, k->samples[i]);
        vec3 sp  = vec3_add(frag, vec3_scale(dir, prm->radius));

        // project to find the screen pixel this sample lands on
        vec2 uv;
        float ndc_depth;
        if (!ssaox_project(prm->proj, sp, &uv, &ndc_depth)) continue;
        if (uv.x < 0.0f || uv.x > 1.0f || uv.y < 0.0f || uv.y > 1.0f) continue;

        int sx = (int)(uv.x * (float)prm->w);
        int sy = (int)(uv.y * (float)prm->h);
        if (sx < 0) sx = 0; if (sx >= prm->w) sx = prm->w - 1;
        if (sy < 0) sy = 0; if (sy >= prm->h) sy = prm->h - 1;

        // the actual scene surface stored at that screen pixel
        float scene_z = pos[sy * prm->w + sx].z;

        occlusion += ssaox_occlusion_term(prm->radius, prm->bias,
                                           sp.z, scene_z);
        counted++;
    }

    if (counted == 0) return 1.0f;
occlusion = occlusion / (float)counted;
float lit = 1.0f - occlusion;
lit = clamp01(lit);
if (prm->power != 1.0f) lit = powf(lit, prm->power);
return lit;
