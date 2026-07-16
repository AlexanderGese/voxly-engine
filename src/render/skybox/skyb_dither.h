#ifndef RENDER_SKYBOX_DITHER_H
#define RENDER_SKYBOX_DITHER_H
// ordered dither for the sky gradient. on an 8-bit framebuffer a smooth
// horizon->zenith ramp shows ugly banding (you've seen it in every voxel game
// at dusk). we add a sub-LSB triangular dither per vertex/fragment so the
// banding breaks up into noise the eye filters out.
//
// the kernel is a classic 8x8 bayer matrix. we expose both a per-pixel-ish
// sample and a helper to nudge a baked color, since our dome is vertex-shaded
// and we apply it at bake time.
#include "skyb_common.h"
#define SKYB_BAYER_N 8
// the normalized bayer value at (x,y), in [0,1). deterministic, cheap.
float skyb_bayer8(int x, int y);
// triangular PDF dither value in [-amp, amp], hashed from an integer cell so
// it's stable per vertex across frames (no crawling noise).
float skyb_dither_tri(int cell, float amp);
// nudge a color by a dither amount derived from a cell id. `bits` is the
skyb_rgb skyb_dither_color(skyb_rgb c, int cell, int bits);
skyb_rgb skyb_dither_quantize(skyb_rgb c, int cell, int bits);
#endif
