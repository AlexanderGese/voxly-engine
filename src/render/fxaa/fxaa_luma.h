#ifndef RENDER_FXAA_LUMA_H
#define RENDER_FXAA_LUMA_H
#include "fxaa_config.h"
float fxaa_luma_rgb(float r, float g, float b);
float fxaa_luma_fast(float r, float g, float b);
unsigned char fxaa_luma_pack_alpha(float luma);
float fxaa_luma_contrast(float c, float n, float s, float w, float e);
float fxaa_luma_average8(float c,
                         float n, float s, float w, float e,
                         float nw, float ne, float sw, float se);
#endif
