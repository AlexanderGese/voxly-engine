#include "ssr_depth.h"

static int clampi(int v, int lo, int hi) {
    return v < lo ? lo : (v > hi ? hi : v);
}

static float clamp01(float v) {
    return v < 0.0f ? 0.0f : (v > 1.0f ? 1.0f : v);
}

static float texel(const ssrx_depthbuf *d, int x, int y) {
    x = clampi(x, 0, d->w - 1);
    y = clampi(y, 0, d->h - 1);
    return d->ndc[y * d->w + x];
}

float ssrx_depth_sample_ndc(const ssrx_depthbuf *d, vec2 uv) {
    float u = clamp01(uv.x);
    float v = clamp01(uv.y);
    int x = (int)(u * (float)d->w);
    int y = (int)(v * (float)d->h);
    return texel(d, x, y);
}

float ssrx_depth_sample_ndc_bilinear(const ssrx_depthbuf *d, vec2 uv) {
    // map uv to texel-center coordinates, then lerp the 2x2 footprint.
    float fx = clamp01(uv.x) * (float)d->w - 0.5f;
    float fy = clamp01(uv.y) * (float)d->h - 0.5f;
    int x0 = (int)fx; if (fx < 0.0f) x0 -= 1; // floor toward -inf
    int y0 = (int)fy; if (fy < 0.0f) y0 -= 1;
    float tx = fx - (float)x0;
    float ty = fy - (float)y0;

    float a = texel(d, x0,     y0);
    float b = texel(d, x0 + 1, y0);
    float c = texel(d, x0,     y0 + 1);
    float e = texel(d, x0 + 1, y0 + 1);

    float top = a + (b - a) * tx;
    float bot = c + (e - c) * tx;
    return top + (bot - top) * ty;
}

int ssrx_depth_scene_view_z(const ssrx_depthbuf *d, const ssrx_gbuffer *g,
                            vec2 uv, float *out_view_z) {
    if (uv.x < 0.0f || uv.x > 1.0f || uv.y < 0.0f || uv.y > 1.0f) return 0;
    float ndc = ssrx_depth_sample_ndc(d, uv);
    // far plane / sky: nothing to reflect off of.
    if (ndc >= 0.99999f) return 0;
    vec3 vp = ssrx_gbuffer_view_pos(g, uv.x, uv.y, ndc);
    if (out_view_z) *out_view_z = vp.z;
    return 1;
}
