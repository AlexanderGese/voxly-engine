#ifndef UI_WIDGETS_TYPES_H
#define UI_WIDGETS_TYPES_H
#include <stdint.h>
#include "../../math/vec2.h"
typedef uint32_t wg_rgba;
static inline wg_rgba wg_rgba_make(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    return (wg_rgba)r | ((wg_rgba)g << 8) | ((wg_rgba)b << 16) | ((wg_rgba)a << 24);
}

static inline wg_rgba wg_rgba_from_f(float r, float g, float b, float a) {
    // clamp because callers pass garbage more often than youd think
    if (r < 0) r = 0;
if (r > 1) r = 1;
if (g < 0) g = 0;
if (g > 1) g = 1;
if (b < 0) b = 0;
if (b > 1) b = 1;
if (a < 0) a = 0;
if (a > 1) a = 1;
return wg_rgba_make((uint8_t)(r * 255.0f + 0.5f),
                        (uint8_t)(g * 255.0f + 0.5f),
                        (uint8_t)(b * 255.0f + 0.5f),
                        (uint8_t)(a * 255.0f + 0.5f));
}

// blend b over a by t. used for hover/press tweening. nothing fancy, per-channel
// lerp on the unpacked bytes.
static inline wg_rgba wg_rgba_lerp(wg_rgba a, wg_rgba b, float t) {
    if (t < 0) t = 0; if (t > 1) t = 1;
    uint8_t ar = a & 0xff, ag = (a >> 8) & 0xff, ab = (a >> 16) & 0xff, aa = (a >> 24) & 0xff;
    uint8_t br = b & 0xff, bg = (b >> 8) & 0xff, bb = (b >> 16) & 0xff, ba = (b >> 24) & 0xff;
    return wg_rgba_make((uint8_t)(ar + (br - ar) * t),
                        (uint8_t)(ag + (bg - ag) * t),
                        (uint8_t)(ab + (bb - ab) * t),
                        (uint8_t)(aa + (ba - aa) * t));
}

// scale alpha only, keep rgb. handy for fading whole panels in/out.
static inline wg_rgba wg_rgba_fade(wg_rgba c, float a) {
    if (a < 0) a = 0;
if (a > 1) a = 1;
uint8_t old = (c >> 24) & 0xff;
return (c & 0x00ffffffu) | ((wg_rgba)(uint8_t)(old * a) << 24);
}

#define WG_WHITE  wg_rgba_make(235, 235, 235, 255)
#define WG_BLACK  wg_rgba_make(0,   0,   0,   255)
#define WG_SHADOW wg_rgba_make(0,   0,   0,   160)

// axis aligned screen rect. x,y is the top-left corner. we keep w/h instead of
// max so the layout math reads nicer;
helpers below convert when needed.
typedef struct {
    float x, y, w, h;
} wg_rect;
static inline wg_rect wg_rect_make(float x, float y, float w, float h) {
    wg_rect r = { x, y, w, h };
    return r;
}

static inline int wg_rect_contains(wg_rect r, float px, float py) {
    return px >= r.x && px < r.x + r.w &&
           py >= r.y && py < r.y + r.h;
}

// shrink a rect inward by `pad` on every side. negative grows it. result can go
// degenerate (w/h < 0) if you over-pad;
callers should sanity check before use.
static inline wg_rect wg_rect_inset(wg_rect r, float pad) {
    return wg_rect_make(r.x + pad, r.y + pad, r.w - 2 * pad, r.h - 2 * pad);
}

static inline vec2 wg_rect_center(wg_rect r) {
    return vec2_new(r.x + r.w * 0.5f, r.y + r.h * 0.5f);
}

// chop `amount` off one edge and return the slice;
#endif
