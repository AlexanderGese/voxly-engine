#ifndef RENDER_TEXT_COMMON_H
#define RENDER_TEXT_COMMON_H

// shared little types for the text/ subsystem. the old render/text.c was a
// fixed-width 6x8 thing with no kerning and no wrapping — fine for the early
// hud but it fell over the moment i wanted proportional glyphs and multi-line
// tooltips. so this is the second pass. everything here is screen-space, in
// pixels, top-left origin (y grows down) to match how the hud thinks.

#include <stdint.h>
#include "../gl.h"

// codepoints we actually support. plain ascii printable range plus a couple of
// box-drawing slots we stuff in at the top. anything outside maps to glyph 0
// (the tofu box) on lookup.
#define TEXT_FIRST_CP    32
#define TEXT_LAST_CP     126
#define TEXT_GLYPH_COUNT (TEXT_LAST_CP - TEXT_FIRST_CP + 1)

// max kerning pairs we keep per font. proportional fonts rarely need more than
// a few dozen real pairs, the rest is noise. if a font wants more it gets
// truncated and nobody dies.
#define TEXT_MAX_KERN_PAIRS  256

// how big a single batch flush can get before we force an upload. 8k quads is
// ~32k verts which is plenty for any sane frame of hud text.
#define TEXT_BATCH_MAX_QUADS 8192

// rgba8 packed the way the shader wants it: 0xAABBGGRR (little endian so the
// bytes land r,g,b,a in memory). keeps the vertex small.
typedef uint32_t text_rgba;

static inline text_rgba text_rgba_make(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    return (text_rgba)r | ((text_rgba)g << 8) | ((text_rgba)b << 16) | ((text_rgba)a << 24);
}

static inline text_rgba text_rgba_from_f(float r, float g, float b, float a) {
    // clamp because callers pass garbage more often than youd think
    if (r < 0) r = 0; if (r > 1) r = 1;
    if (g < 0) g = 0; if (g > 1) g = 1;
    if (b < 0) b = 0; if (b > 1) b = 1;
    if (a < 0) a = 0; if (a > 1) a = 1;
    return text_rgba_make((uint8_t)(r * 255.0f + 0.5f),
                          (uint8_t)(g * 255.0f + 0.5f),
                          (uint8_t)(b * 255.0f + 0.5f),
                          (uint8_t)(a * 255.0f + 0.5f));
}

#define TEXT_WHITE  text_rgba_make(255, 255, 255, 255)
#define TEXT_BLACK  text_rgba_make(0,   0,   0,   255)
#define TEXT_SHADOW text_rgba_make(0,   0,   0,   160)

// one vertex in the text batch. pos in screen px, uv in atlas 0..1, color
// packed. matches the attrib layout set up in text_batch.c — keep them in sync.
typedef struct {
    float     x, y;
    float     u, v;
    text_rgba color;
} text_vertex;

// horizontal alignment for a laid-out block
typedef enum {
    TEXT_ALIGN_LEFT = 0,
    TEXT_ALIGN_CENTER,
    TEXT_ALIGN_RIGHT
} text_align;

#endif
