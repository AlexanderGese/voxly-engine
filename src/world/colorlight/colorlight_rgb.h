#ifndef WORLD_COLORLIGHT_RGB_H
#define WORLD_COLORLIGHT_RGB_H

#include <stdint.h>

// the working color type. three bytes, 0..255 per channel. this is the form
// the mesher and the emitter table speak. the flood fill itself works in
// 0..MAX_LIGHT levels (see colorlight_grid) and only widens to this at the
// edges. kept dead simple on purpose, no alpha, no gamma, that's blend's job.

typedef struct {
    uint8_t r, g, b;
} colorlight_rgb;

static const colorlight_rgb COLORLIGHT_BLACK = {0, 0, 0};
static const colorlight_rgb COLORLIGHT_WHITE = {255, 255, 255};

colorlight_rgb colorlight_rgb_new(uint8_t r, uint8_t g, uint8_t b);

// per-channel max / add, both saturating at 255. used when two light sources
// overlap: brightest channel wins (max), not additive blowout.
colorlight_rgb colorlight_rgb_max(colorlight_rgb a, colorlight_rgb b);
colorlight_rgb colorlight_rgb_add(colorlight_rgb a, colorlight_rgb b);

// scale all channels by s in [0,1]. rounds, doesn't floor, so a dim torch
// doesn't drop a channel to zero one step early.
colorlight_rgb colorlight_rgb_scale(colorlight_rgb a, float s);

// channel-wise multiply, both treated as 0..1. this is how a colored light
// gets modulated by a colored sun tint at blend time.
colorlight_rgb colorlight_rgb_mul(colorlight_rgb a, colorlight_rgb b);

// linear interp, t in [0,1]. for smooth-stepping light over a frame or two.
colorlight_rgb colorlight_rgb_lerp(colorlight_rgb a, colorlight_rgb b, float t);

// brightest single channel, 0..255. used to early-out floods (if the strongest
// channel can't reach a neighbor, none can).
uint8_t colorlight_rgb_peak(colorlight_rgb a);

// rough perceptual luma, 0..255. rec601 weights. handy for debug overlays and
// for deciding whether a light is "basically off".
uint8_t colorlight_rgb_luma(colorlight_rgb a);

int colorlight_rgb_equal(colorlight_rgb a, colorlight_rgb b);
int colorlight_rgb_is_black(colorlight_rgb a);

#endif
