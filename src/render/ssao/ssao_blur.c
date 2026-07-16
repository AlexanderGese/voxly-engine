#include "ssao_blur.h"
#include "ssao_config.h"
#include "../../util/log.h"

#include <stddef.h>

static glid make_r8_tex(int w, int h) {
    glid t;
    glGenTextures(1, &t);
    glBindTexture(GL_TEXTURE_2D, t);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, w, h, 0, GL_RED, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_2D, 0);
    return t;
}

int ssaox_blur_init(ssaox_blur *b, int w, int h, glid prog) {
    b->w = w;
    b->h = h;
    b->radius = SSAOX_BLUR_RADIUS;
    b->prog = prog;

    b->tex = make_r8_tex(w, h);
    glGenFramebuffers(1, &b->fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, b->fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                           GL_TEXTURE_2D, b->tex, 0);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        LOGW("ssaox blur fbo incomplete");
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        return 0;
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    return 1;
}

void ssaox_blur_destroy(ssaox_blur *b) {
    if (b->fbo) glDeleteFramebuffers(1, &b->fbo);
    if (b->tex) glDeleteTextures(1, &b->tex);
    b->fbo = 0;
    b->tex = 0;
    // we dont own prog, the parent pass deletes it.
}

void ssaox_blur_resize(ssaox_blur *b, int w, int h) {
    if (b->w == w && b->h == h) return;
    glid prog = b->prog;
    ssaox_blur_destroy(b);
    ssaox_blur_init(b, w, h, prog);
}

glid ssaox_blur_run(ssaox_blur *b, glid occl_tex, glid fs_vao) {
    if (!b->prog) return occl_tex;  // no shader, hand back the raw buffer

    glBindFramebuffer(GL_FRAMEBUFFER, b->fbo);
    glViewport(0, 0, b->w, b->h);
    glUseProgram(b->prog);

    glActiveTexture(GL_TEXTURE0 + SSAOX_TEX_UNIT_OCCL);
    glBindTexture(GL_TEXTURE_2D, occl_tex);
    gl_set_uniform_int(b->prog, "u_occlusion", SSAOX_TEX_UNIT_OCCL);
    gl_set_uniform_int(b->prog, "u_radius", b->radius);
    gl_set_uniform_float(b->prog, "u_texel_x", 1.0f / (float)b->w);
    gl_set_uniform_float(b->prog, "u_texel_y", 1.0f / (float)b->h);

    glBindVertexArray(fs_vao);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glBindVertexArray(0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    return b->tex;
}

void ssaox_blur_cpu(const float *in, float *out, int w, int h, int radius) {
    if (radius < 0) radius = 0;
    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            float sum = 0.0f;
            int n = 0;
            for (int dy = -radius; dy <= radius; dy++) {
                int sy = y + dy;
                if (sy < 0 || sy >= h) continue;   // clamp by skipping edges
                for (int dx = -radius; dx <= radius; dx++) {
                    int sx = x + dx;
                    if (sx < 0 || sx >= w) continue;
                    sum += in[sy * w + sx];
                    n++;
                }
            }
            out[y * w + x] = (n > 0) ? sum / (float)n : in[y * w + x];
        }
    }
}
