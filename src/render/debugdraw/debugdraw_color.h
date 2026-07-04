#ifndef RENDER_DEBUGDRAW_COLOR_H
#define RENDER_DEBUGDRAW_COLOR_H

#include <stdint.h>

// packed rgba, one byte per channel. i went back and forth on whether to
// just pass vec3 everywhere but lines need alpha for the ttl fade so a
// packed u32 it is. 0xAABBGGRR in memory, matches what the vbo wants.

typedef uint32_t ddcolor;

static inline ddcolor ddcolor_rgba(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    return (ddcolor)r | ((ddcolor)g << 8) | ((ddcolor)b << 16) | ((ddcolor)a << 24);
}

static inline ddcolor ddcolor_rgb(uint8_t r, uint8_t g, uint8_t b) {
    return ddcolor_rgba(r, g, b, 255);
}

// from 0..1 floats. clamps. dont feed it nan, it wont check.
static inline ddcolor ddcolor_rgbf(float r, float g, float b, float a) {
    if (r < 0) r = 0; if (r > 1) r = 1;
    if (g < 0) g = 0; if (g > 1) g = 1;
    if (b < 0) b = 0; if (b > 1) b = 1;
    if (a < 0) a = 0; if (a > 1) a = 1;
    return ddcolor_rgba((uint8_t)(r * 255.0f + 0.5f),
                        (uint8_t)(g * 255.0f + 0.5f),
                        (uint8_t)(b * 255.0f + 0.5f),
                        (uint8_t)(a * 255.0f + 0.5f));
}

static inline uint8_t ddcolor_r(ddcolor c) { return (uint8_t)(c & 0xff); }
static inline uint8_t ddcolor_g(ddcolor c) { return (uint8_t)((c >> 8)  & 0xff); }
static inline uint8_t ddcolor_b(ddcolor c) { return (uint8_t)((c >> 16) & 0xff); }
static inline uint8_t ddcolor_a(ddcolor c) { return (uint8_t)((c >> 24) & 0xff); }

// scale only the alpha channel. used by the ttl fade so old lines dim out
// instead of popping. t is 0..1.
static inline ddcolor ddcolor_fade(ddcolor c, float t) {
    if (t < 0) t = 0; if (t > 1) t = 1;
    uint32_t a = (uint32_t)(ddcolor_a(c) * t + 0.5f);
    return (c & 0x00ffffffu) | (a << 24);
}

// linear blend between two colors, per channel. nothing clever.
static inline ddcolor ddcolor_lerp(ddcolor a, ddcolor b, float t) {
    if (t < 0) t = 0; if (t > 1) t = 1;
    float it = 1.0f - t;
    return ddcolor_rgba(
        (uint8_t)(ddcolor_r(a) * it + ddcolor_r(b) * t),
        (uint8_t)(ddcolor_g(a) * it + ddcolor_g(b) * t),
        (uint8_t)(ddcolor_b(a) * it + ddcolor_b(b) * t),
        (uint8_t)(ddcolor_a(a) * it + ddcolor_a(b) * t));
}

// handful of named colors so call sites read nicely
#define DDCOLOR_WHITE   ddcolor_rgb(255, 255, 255)
#define DDCOLOR_BLACK   ddcolor_rgb(0,   0,   0)
#define DDCOLOR_RED     ddcolor_rgb(220, 50,  47)
#define DDCOLOR_GREEN   ddcolor_rgb(60,  200, 70)
#define DDCOLOR_BLUE    ddcolor_rgb(70,  120, 230)
#define DDCOLOR_YELLOW  ddcolor_rgb(240, 220, 40)
#define DDCOLOR_CYAN    ddcolor_rgb(40,  220, 220)
#define DDCOLOR_MAGENTA ddcolor_rgb(220, 40,  220)
#define DDCOLOR_ORANGE  ddcolor_rgb(240, 150, 30)
#define DDCOLOR_GRAY    ddcolor_rgb(140, 140, 140)

#endif
