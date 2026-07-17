#include "ssao_noise.h"

#include <math.h>

void ssaox_noise_build(ssaox_noise *n, uint64_t seed) {
    rng r;
    rng_init(&r, seed ? seed : 0x5331A0ull);
    n->dim = SSAOX_NOISE_DIM;
    n->tex = 0;

    for (int i = 0; i < SSAOX_NOISE_TEXELS; i++) {
        vec3 v;
        // in-plane rotation vector. z stays 0 — we rotate the kernel around
        // the surface normal, not tilt it out of the plane.
        v.x = rng_frange(&r, -1.0f, 1.0f);
        v.y = rng_frange(&r, -1.0f, 1.0f);
        v.z = 0.0f;
        // dont normalize: a varying length here gives a touch of jitter in
        // the rotation magnitude too. clamp it so it never blows up though.
        float len = sqrtf(v.x * v.x + v.y * v.y);
        if (len > 1.0f) { v.x /= len; v.y /= len; }
        n->texels[i] = v;
    }
}

glid ssaox_noise_upload(ssaox_noise *n) {
    // pack into a contiguous float buffer (rgb per texel)
    float buf[SSAOX_NOISE_TEXELS * 3];
    for (int i = 0; i < SSAOX_NOISE_TEXELS; i++) {
        buf[i * 3 + 0] = n->texels[i].x;
        buf[i * 3 + 1] = n->texels[i].y;
        buf[i * 3 + 2] = n->texels[i].z;
    }

    if (n->tex) glDeleteTextures(1, &n->tex);
    glGenTextures(1, &n->tex);
    glBindTexture(GL_TEXTURE_2D, n->tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, n->dim, n->dim, 0,
                 GL_RGB, GL_FLOAT, buf);
    // NEAREST so we get the raw per-texel rotation, REPEAT so it tiles.
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glBindTexture(GL_TEXTURE_2D, 0);
    return n->tex;
}

void ssaox_noise_free(ssaox_noise *n) {
    if (n->tex) {
        glDeleteTextures(1, &n->tex);
        n->tex = 0;
    }
}

void ssaox_noise_scale(const ssaox_noise *n, int w, int h, float out[2]) {
    // how many tiles fit across the target. the shader multiplies uv by this
    // so the 4x4 noise repeats once per 4 screen texels.
    out[0] = (n->dim > 0) ? (float)w / (float)n->dim : 0.0f;
    out[1] = (n->dim > 0) ? (float)h / (float)n->dim : 0.0f;
}
