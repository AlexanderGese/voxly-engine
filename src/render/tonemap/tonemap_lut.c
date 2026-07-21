#include "tonemap_lut.h"
#include "../../util/log.h"

#include <stdlib.h>
#include <string.h>
#include <math.h>

static float clampf(float v, float lo, float hi) {
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}

static int clampi(int v, int lo, int hi) {
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}

// flat index of (r,g,b) in the rgb-major layout, *3 for the triple base.
static size_t lut_index(int dim, int r, int g, int b) {
    return ((size_t)((b * dim + g) * dim + r)) * 3u;
}

void tonemap_lut_init(tonemap_lut *l) {
    l->dim   = 0;
    l->data  = NULL;
    l->tex   = 0;
    l->dirty = 0;
}

int tonemap_lut_make_identity(tonemap_lut *l, int dim) {
    tonemap_lut_init(l);
    dim = clampi(dim, 2, TONEMAP_LUT_MAX_DIM);

    size_t count = (size_t)dim * dim * dim;
    l->data = (float *)malloc(count * 3u * sizeof(float));
    if (!l->data) {
        LOGE("tonemap: lut alloc failed for dim %d", dim);
        return 0;
    }
    l->dim = dim;

    float inv = 1.0f / (float)(dim - 1);
    for (int b = 0; b < dim; b++) {
        for (int g = 0; g < dim; g++) {
            for (int r = 0; r < dim; r++) {
                size_t i = lut_index(dim, r, g, b);
                l->data[i + 0] = r * inv;
                l->data[i + 1] = g * inv;
                l->data[i + 2] = b * inv;
            }
        }
    }
    l->dirty = 1;
    return 1;
}

void tonemap_lut_destroy(tonemap_lut *l) {
    if (l->data) free(l->data);
    if (l->tex)  glDeleteTextures(1, &l->tex);
    tonemap_lut_init(l);
}

void tonemap_lut_set(tonemap_lut *l, int r, int g, int b, vec3 color) {
    if (!l->data || l->dim <= 0) return;
    r = clampi(r, 0, l->dim - 1);
    g = clampi(g, 0, l->dim - 1);
    b = clampi(b, 0, l->dim - 1);
    size_t i = lut_index(l->dim, r, g, b);
    l->data[i + 0] = color.x;
    l->data[i + 1] = color.y;
    l->data[i + 2] = color.z;
    l->dirty = 1;
}

// fetch a grid node as a vec3, no interpolation.
static vec3 lut_node(const tonemap_lut *l, int r, int g, int b) {
    size_t i = lut_index(l->dim, r, g, b);
    return vec3_new(l->data[i + 0], l->data[i + 1], l->data[i + 2]);
}

vec3 tonemap_lut_sample(const tonemap_lut *l, vec3 c) {
    if (!l->data || l->dim < 2) return c;

    float scale = (float)(l->dim - 1);
    float fr = clampf(c.x, 0.0f, 1.0f) * scale;
    float fg = clampf(c.y, 0.0f, 1.0f) * scale;
    float fb = clampf(c.z, 0.0f, 1.0f) * scale;

    int r0 = (int)floorf(fr), g0 = (int)floorf(fg), b0 = (int)floorf(fb);
    int r1 = clampi(r0 + 1, 0, l->dim - 1);
    int g1 = clampi(g0 + 1, 0, l->dim - 1);
    int b1 = clampi(b0 + 1, 0, l->dim - 1);
    r0 = clampi(r0, 0, l->dim - 1);
    g0 = clampi(g0, 0, l->dim - 1);
    b0 = clampi(b0, 0, l->dim - 1);

    float dr = fr - (float)r0;
    float dg = fg - (float)g0;
    float db = fb - (float)b0;

    // trilerp: 4 lerps along r, 2 along g, 1 along b. the textbook unrolling.
    vec3 c00 = vec3_lerp(lut_node(l, r0, g0, b0), lut_node(l, r1, g0, b0), dr);
    vec3 c10 = vec3_lerp(lut_node(l, r0, g1, b0), lut_node(l, r1, g1, b0), dr);
    vec3 c01 = vec3_lerp(lut_node(l, r0, g0, b1), lut_node(l, r1, g0, b1), dr);
    vec3 c11 = vec3_lerp(lut_node(l, r0, g1, b1), lut_node(l, r1, g1, b1), dr);

    vec3 c0 = vec3_lerp(c00, c10, dg);
    vec3 c1 = vec3_lerp(c01, c11, dg);
    return vec3_lerp(c0, c1, db);
}

int tonemap_lut_blend(tonemap_lut *dst, const tonemap_lut *a,
                      const tonemap_lut *b, float t) {
    if (!a->data || !b->data || a->dim != b->dim) {
        LOGW("tonemap: lut blend dim mismatch (%d vs %d)",
             a ? a->dim : -1, b ? b->dim : -1);
        return 0;
    }
    if (dst->dim != a->dim || !dst->data) {
        tonemap_lut_destroy(dst);
        if (!tonemap_lut_make_identity(dst, a->dim)) return 0;
    }
    t = clampf(t, 0.0f, 1.0f);

    size_t n = (size_t)a->dim * a->dim * a->dim * 3u;
    for (size_t i = 0; i < n; i++)
        dst->data[i] = a->data[i] + (b->data[i] - a->data[i]) * t;

    dst->dirty = 1;
    return 1;
}

glid tonemap_lut_upload(tonemap_lut *l) {
    if (!l->data || l->dim < 2) return 0;
    if (!l->dirty && l->tex) return l->tex;

    if (!l->tex) {
        glGenTextures(1, &l->tex);
        glBindTexture(GL_TEXTURE_3D, l->tex);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        // clamp on every axis so edge colors dont wrap into their opposite.
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    } else {
        glBindTexture(GL_TEXTURE_3D, l->tex);
    }

    glTexImage3D(GL_TEXTURE_3D, 0, GL_RGB16F, l->dim, l->dim, l->dim, 0,
                 GL_RGB, GL_FLOAT, l->data);
    glBindTexture(GL_TEXTURE_3D, 0);

    l->dirty = 0;
    return l->tex;
}
