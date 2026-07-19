#ifndef RENDER_TEXT_LAYOUT_H
#define RENDER_TEXT_LAYOUT_H

// shaping + layout. turns a utf8-ish (really latin1/ascii) string into a list
// of positioned glyphs, handling kerning, tabs, explicit '\n', and optional
// word wrapping at a max width. the result is a flat array of placements that
// the batch builder consumes. no bidi, no script shaping, no ligatures — this
// is a voxel game hud, not a word processor.

#include "text_font.h"
#include "text_common.h"

// one positioned glyph. pos is the top-left of the glyph quad in screen px.
typedef struct {
    const text_glyph *g;
    float x, y;         // quad top-left
    int   line;         // which line it landed on (0-based)
} text_placement;

// a laid-out paragraph. placements is heap-allocated and owned by the result;
// call text_layout_free when done. width/height are the tight bounding box.
typedef struct {
    text_placement *items;   // darray (util/darray.h)
    int   count;
    int   line_count;
    float width;             // widest line
    float height;            // total block height
    float line_height;       // copied from font for convenience
} text_layout;

// params for a layout pass. zero-init then set what you care about.
typedef struct {
    float      max_width;    // wrap width in px, <=0 disables wrapping
    text_align align;        // horizontal alignment within max_width (or width)
    float      line_spacing; // multiplier on font line_height (1.0 = default)
    int        tab_cols;     // override font tab stop, <=0 uses font's
} text_layout_opts;

// lay out `s` with `font` starting at the origin (0,0). caller can translate
// the whole block afterwards (the batch builder takes an offset). returns 1 on
// success; on failure `out` is zeroed.
int  text_layout_run(const text_font *font, const char *s,
                     const text_layout_opts *opts, text_layout *out);

void text_layout_free(text_layout *l);

// cheap measure: just the bounding box, no placement array kept. handy for hud
// code that needs to size a panel before deciding where to draw.
void text_layout_measure(const text_font *font, const char *s,
                         const text_layout_opts *opts,
                         float *out_w, float *out_h);

#endif
