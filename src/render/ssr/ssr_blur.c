#include "ssr_blur.h"
#include "../../util/log.h"

#include <stddef.h>

static int clampi(int v, int lo, int hi) {
    return v < lo ? lo : (v > hi ? hi : v);
}

static float clamp01(float v) {
    return v < 0.0f ? 0.0f : (v > 1.0f ? 1.0f : v);
}

static glid make_tex(int w, int h) {
    glid t;
    glGenTextures(1, &t);
    glBindTexture(GL_TEXTURE_2D, t);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, w, h, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_2D, 0);
    return t;
}

int ssrx_blur_init(ssrx_blur *b, int w, int h, glid prog, int max_radius) {
    b->w = w;
    b->h = h;
    b->prog = prog;
    b->max_radius = max_radius < 0 ? 0 : max_radius;

    b->tex = make_tex(w, h);
    glGenFramebuffers(1, &b->fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, b->fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                           GL_TEXTURE_2D, b->tex, 0);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        LOGW("ssrx blur fbo incomplete");
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    return (b->tex != 0);
}

void ssrx_blur_destroy(ssrx_blur *b) {
    if (b->fbo) glDeleteFramebuffers(1, &b->fbo);
    if (b->tex) glDeleteTextures(1, &b->tex);
    b->fbo = b->tex = 0;
}

void ssrx_blur_resize(ssrx_blur *b, int w, int h) {
    if (b->w == w && b->h == h) return;
    b->w = w;
    b->h = h;
    if (b->tex) glDeleteTextures(1, &b->tex);
    b->tex = make_tex(w, h);
    glBindFramebuffer(GL_FRAMEBUFFER, b->fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                           GL_TEXTURE_2D, b->tex, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

int ssrx_blur_radius_for(int max_radius, float roughness) {
    float r = clamp01(roughness);
    // quadratic ramp: smooth surfaces ~0, only rough ones spread to max.
    float scaled = r * r * (float)max_radius;
    int rad = (int)(scaled + 0.5f);
    return clampi(rad, 0, max_radius);
}

glid ssrx_blur_run(ssrx_blur *b, glid reflect_tex, glid rough_tex, glid fs_vao) {
    if (!b->prog) return reflect_tex;   // no shader — hand back the input

    glBindFramebuffer(GL_FRAMEBUFFER, b->fbo);
    glViewport(0, 0, b->w, b->h);
    glUseProgram(b->prog);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, reflect_tex);
    gl_set_uniform_int(b->prog, "u_reflect", 0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, rough_tex);
    gl_set_uniform_int(b->prog, "u_rough", 1);
    gl_set_uniform_int(b->prog, "u_has_rough", rough_tex ? 1 : 0);

    gl_set_uniform_int(b->prog, "u_max_radius", b->max_radius);
    gl_set_uniform_float(b->prog, "u_texel_x", 1.0f / (float)b->w);
    gl_set_uniform_float(b->prog, "u_texel_y", 1.0f / (float)b->h);

    glBindVertexArray(fs_vao);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glBindVertexArray(0);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    return b->tex;
}

void ssrx_blur_cpu(const vec4 *in, const float *rough, vec4 *out,
                   int w, int h, int max_radius) {
    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            int idx = y * w + x;
            int rad = rough ? ssrx_blur_radius_for(max_radius, rough[idx])
                            : max_radius;

            // confidence-weighted accumulation. weight each tap by its alpha so
            // empty (alpha 0) neighbours dont drag a valid reflection toward
            // black — that would halo the reflection edges.
            float ar = 0.0f, ag = 0.0f, ab = 0.0f, aa = 0.0f;
            float wsum = 0.0f;

            for (int dy = -rad; dy <= rad; dy++) {
                int sy = clampi(y + dy, 0, h - 1);
                for (int dx = -rad; dx <= rad; dx++) {
                    int sx = clampi(x + dx, 0, w - 1);
                    vec4 s = in[sy * w + sx];
                    float wgt = s.w;          // confidence as the weight
                    ar += s.x * wgt;          // s.xyz is already premultiplied
                    ag += s.y * wgt;
                    ab += s.z * wgt;
                    aa += s.w * wgt;
                    wsum += wgt;
                }
            }

            if (wsum > 1e-6f) {
                float inv = 1.0f / wsum;
                out[idx] = vec4_new(ar * inv, ag * inv, ab * inv, aa * inv);
            } else {
                // no confident taps in the footprint — pass the center through.
                out[idx] = in[idx];
            }
        }
    }
}
