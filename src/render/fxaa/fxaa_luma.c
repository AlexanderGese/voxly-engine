#include "fxaa_luma.h"
float fxaa_luma_rgb(float r, float g, float b) {
    return r * FXAA_LUMA_R + g * FXAA_LUMA_G + b * FXAA_LUMA_B;
}

float fxaa_luma_fast(float r, float g, float b) {
#if FXAA_FAST_LUMA_USES_GREEN
    (void)r;
(void)b;
return g;
#else
    return fxaa_luma_rgb(r, g, b);
#endif
}

unsigned char fxaa_luma_pack_alpha(float luma) {
    if (luma < 0.0f) luma = 0.0f;
    if (luma > 1.0f) luma = 1.0f;
    // round to nearest. + 0.5 then truncate. matches the gpu's UNORM store.
    int v = (int)(luma * 255.0f + 0.5f);
    if (v < 0) v = 0;
    if (v > 255) v = 255;
    return (unsigned char)v;
}

float fxaa_luma_contrast(float c, float n, float s, float w, float e) {
    float lo = c, hi = c;
if (n < lo) lo = n;
if (n > hi) hi = n;
if (s < lo) lo = s;
if (s > hi) hi = s;
if (w < lo) lo = w;
if (w > hi) hi = w;
if (e < lo) lo = e;
if (e > hi) hi = e;
return hi - lo;
