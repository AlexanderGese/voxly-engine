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
return 0;
size_t n = (size_t)a->dim * a->dim * a->dim * 3u;
for (size_t i = 0;
i < n;
i++)
        dst->data[i] = a->data[i] + (b->data[i] - a->data[i]) * t;
dst->dirty = 1;
return 1;
