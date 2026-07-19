#ifndef RENDER_TEXT_FONT_H
#define RENDER_TEXT_FONT_H

// a baked font: per-glyph metrics, the atlas they live in, line metrics, and a
// small kerning table. this is deliberately bitmap-only — no freetype, no
// hinting, no subpixel. the engine ships one embedded pixel font and that's it.
// (render/font.* still has the original 6x8 fixed thing; this supersedes it
// for anything that wants proportional spacing.)

#include "text_atlas.h"
#include "text_common.h"

// metrics for one glyph, all in pixels. origin is the pen position sitting on
// the baseline. bearings are how far the bitmap is offset from that pen.
typedef struct {
    text_atlas_rect rect;   // where it lives in the atlas (0 w/h => no bitmap)
    int   advance;          // how far to move the pen after drawing
    int   bearing_x;        // left side bearing
    int   bearing_y;        // top of bitmap above baseline (positive = up)
    int   w, h;             // bitmap size, mirrors rect but cached
} text_glyph;

// one kerning adjustment for an ordered pair (left, right).
typedef struct {
    uint8_t left;
    uint8_t right;
    int8_t  amount;         // added to the left glyph's advance, usually <0
} text_kern;

typedef struct {
    text_atlas atlas;       // owns its own atlas. one font, one atlas.
    text_glyph glyphs[TEXT_GLYPH_COUNT];

    // a tiny fully-opaque white rect baked into the atlas. lets panel/box code
    // draw solid quads through the same text shader + draw call.
    text_atlas_rect white;

    int ascent;             // baseline to top of line box
    int descent;            // baseline to bottom (positive number)
    int line_gap;           // extra leading between lines
    int line_height;        // ascent + descent + line_gap, cached
    int space_advance;      // advance for ' ', also used for tab math
    int tab_cols;           // a tab snaps to multiples of this * space_advance

    text_kern kern[TEXT_MAX_KERN_PAIRS];
    int       kern_count;

    int baked;              // glyphs + atlas populated & uploaded
} text_font;

// build the engine's default embedded pixel font into `f`. allocates an atlas,
// rasterizes the bitmaps, sets metrics, uploads. returns 1 on success.
int  text_font_init_builtin(text_font *f);
void text_font_destroy(text_font *f);

// glyph lookup. out-of-range or missing codepoints fall back to glyph index 0.
const text_glyph *text_font_glyph(const text_font *f, uint32_t cp);

// kerning between two codepoints. 0 if no entry. linear scan — the table is
// tiny and this is called in the layout hot path but stays cache-friendly.
int  text_font_kern(const text_font *f, uint32_t left, uint32_t right);

// register a kerning pair while baking (ignored once the table is full).
void text_font_add_kern(text_font *f, uint32_t left, uint32_t right, int amount);

#endif
