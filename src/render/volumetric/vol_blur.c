#include "vol_blur.h"

#include <math.h>
#include <stddef.h>

void volumetric_blur_build(volumetric_blur *b) {
    b->radius = VOL_BLUR_RADIUS;
    b->taps = 2 * b->radius + 1;
    b->depth_sigma = VOL_BLUR_DEPTH_SIGMA;

    // sigma ~ radius/2 gives a kernel that's mostly faded by the edge tap.
    float sigma = (b->radius > 0) ? (float)b->radius * 0.5f : 1.0f;
    float inv2s2 = 1.0f / (2.0f * sigma * sigma);

    float sum = 0.0f;
    for (int i = -b->radius; i <= b->radius; i++) {
        float w = expf(-(float)(i * i) * inv2s2);
        b->weights[i + b->radius] = w;
        sum += w;
    }
    // normalize so a flat input survives the blur unchanged.
    if (sum > 0.0f) {
        for (int i = 0; i < b->taps; i++) b->weights[i] /= sum;
    }
}

int volumetric_blur_pack(const volumetric_blur *b, float *out) {
    // centre + positive side only; the gpu folds the symmetric negative half.
    int n = 0;
    for (int i = b->radius; i < b->taps; i++) out[n++] = b->weights[i];
    return n;   // radius + 1
}

// bilateral depth weight: gaussian on the depth difference. far-apart depths
// contribute almost nothing, which is what keeps shafts off the foreground.
static float depth_weight(float dc, float ds, float sigma) {
    if (sigma <= 0.0f) return 1.0f;
    float diff = (dc - ds) / sigma;
    return expf(-0.5f * diff * diff);
}

int volumetric_blur_run(const volumetric_blur *b,
                        const volumetric_programs *progs,
                        const volumetric_quad *quad,
                        volumetric_target *t,
                        glid depth_tex) {
    if (!progs->ok) return 0;

    float half[VOL_BLUR_RADIUS + 1];
    int hn = volumetric_blur_pack(b, half);

    // shared setup: depth on the scene unit, weights + radius uniforms.
    volumetric_programs_use_blur(progs);
    glActiveTexture(GL_TEXTURE0 + VOL_TEX_UNIT_SCENE);
    glBindTexture(GL_TEXTURE_2D, depth_tex);
    glUniform1fv(glGetUniformLocation(progs->blur, "u_weights"), hn, half);
    gl_set_uniform_int(progs->blur, "u_radius", b->radius);
    gl_set_uniform_float(progs->blur, "u_depth_sigma", b->depth_sigma);

    float texel_x = (t->w > 0) ? 1.0f / (float)t->w : 0.0f;
    float texel_y = (t->h > 0) ? 1.0f / (float)t->h : 0.0f;

    // horizontal: read slot 0, write slot 1
    volumetric_target_bind(t, 1);
    gl_set_uniform_vec3(progs->blur, "u_dir", texel_x, 0.0f, 0.0f);
    glActiveTexture(GL_TEXTURE0 + VOL_TEX_UNIT_SCATTER);
    glBindTexture(GL_TEXTURE_2D, t->tex[0]);
    volumetric_quad_draw(quad);

    // vertical: read slot 1, write slot 0
    volumetric_target_bind(t, 0);
    gl_set_uniform_vec3(progs->blur, "u_dir", 0.0f, texel_y, 0.0f);
    glActiveTexture(GL_TEXTURE0 + VOL_TEX_UNIT_SCATTER);
    glBindTexture(GL_TEXTURE_2D, t->tex[1]);
    volumetric_quad_draw(quad);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    return 1;
}

// one separable bilateral pass along a stride. clamps at the borders.
static void blur_axis(const volumetric_blur *b,
                      const float *src, float *dst,
                      const float *depth, int w, int h,
                      int dx, int dy) {
    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            int ci = y * w + x;
            float dc = depth ? depth[ci] : 0.0f;
            float acc = 0.0f, wsum = 0.0f;
            for (int k = -b->radius; k <= b->radius; k++) {
                int sx = x + dx * k;
                int sy = y + dy * k;
                if (sx < 0) sx = 0; else if (sx >= w) sx = w - 1;
                if (sy < 0) sy = 0; else if (sy >= h) sy = h - 1;
                int si = sy * w + sx;
                float gw = b->weights[k + b->radius];
                float bw = depth ? depth_weight(dc, depth[si], b->depth_sigma)
                                 : 1.0f;
                float ww = gw * bw;
                acc += src[si] * ww;
                wsum += ww;
            }
            dst[ci] = (wsum > 0.0f) ? acc / wsum : src[ci];
        }
    }
}

void volumetric_blur_cpu(const volumetric_blur *b,
                         float *img, float *scratch,
                         const float *depth, int w, int h) {
    // horizontal img -> scratch, then vertical scratch -> img
    blur_axis(b, img, scratch, depth, w, h, 1, 0);
    blur_axis(b, scratch, img, depth, w, h, 0, 1);
}
