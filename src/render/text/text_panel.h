#ifndef RENDER_TEXT_PANEL_H
#define RENDER_TEXT_PANEL_H

// little immediate-mode helpers built on text_ctx: a stacked row list (the
// debug overlay / F3 screen), and a boxed tooltip that auto-sizes to its text.
// these draw a solid background quad behind the text using the same batch — the
// background uses a 1x1 white texel from the font atlas (we reserve one) so it
// still goes through the single text draw call. no separate quad shader needed.

#include "text_ctx.h"
#include "text_common.h"

// a vertical stack of left-aligned rows. begin, push lines, the cursor walks
// down by line_height each push. used for the debug readout in the corner.
typedef struct {
    text_ctx *ctx;
    float x, y;       // top-left where the next row goes
    float start_x;
    float start_y;    // y at begin(), so we can report consumed height
    float row_step;   // line height * spacing
    float widest;     // widest row drawn, for an optional backing box
    int   rows;
} text_panel;

void  text_panel_begin(text_panel *p, text_ctx *c, float x, float y);
// draw one row, advance the cursor. returns the row's width.
float text_panel_row(text_panel *p, const char *s);
float text_panel_rowf(text_panel *p, const char *fmt, ...);
// a row in a custom color
float text_panel_row_color(text_panel *p, const char *s, text_rgba color);
// a blank half-height gap between groups
void  text_panel_gap(text_panel *p);

// total height consumed so far (rows * step + gaps), for sizing a background.
float text_panel_height(const text_panel *p);

// draw a centered tooltip box around `s` at (cx, cy is the box's anchor top).
// auto-sizes with padding, draws a translucent background then the text.
void  text_panel_tooltip(text_ctx *c, const char *s, float cx, float top,
                         text_rgba bg, text_rgba fg);

#endif
