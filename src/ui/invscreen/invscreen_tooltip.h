#ifndef UI_INVSCREEN_TOOLTIP_H
#define UI_INVSCREEN_TOOLTIP_H

// hover tooltip. tracks which slot the cursor is resting on and for how long;
// once the dwell passes INVSCR_TOOLTIP_DELAY it builds a couple of text lines
// (item name, count, a flavor line) that the draw layer renders in a little box
// near the cursor. moving to a different slot resets the timer.

#include "invscreen_model.h"

#define INVSCR_TOOLTIP_LINES   3
#define INVSCR_TOOLTIP_LINELEN 48

typedef struct {
    int   slot;                 // slot currently hovered, INVSCR_NO_SLOT if none
    float dwell;                // seconds the cursor has rested on `slot`
    int   visible;              // dwell passed the delay and slot has content
    int   nlines;
    char  line[INVSCR_TOOLTIP_LINES][INVSCR_TOOLTIP_LINELEN];
} invscreen_tooltip;

void invscreen_tooltip_init(invscreen_tooltip *t);

// feed the slot under the cursor each frame (INVSCR_NO_SLOT when over nothing or
// while dragging). advances/resets the dwell timer and, once it trips, fills the
// text lines from the model. returns 1 when the tooltip is visible this frame.
int invscreen_tooltip_update(invscreen_tooltip *t, const invscreen_model *m,
                             int slot, float dt);

// widest line in pixels for a given text scale and per-glyph advance. the draw
// layer uses this to size the box. counts visible chars, nothing clever.
float invscreen_tooltip_width(const invscreen_tooltip *t, float glyph_w);

#endif
