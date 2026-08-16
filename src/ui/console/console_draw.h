#ifndef UI_CONSOLE_DRAW_H
#define UI_CONSOLE_DRAW_H

// renders the console panel: a translucent backing quad, the scrollback
// rows top-down, a separator, the edit line with a blinking caret, and a
// little scrollbar nub when there's more above. drawing is split out so
// the core has zero render-layer includes.

#include "console.h"
#include "../../render/text.h"

// layout knobs. line height is the font cell + a couple px of leading.
#define CONSOLE_LINE_H     14
#define CONSOLE_PAD        8
#define CONSOLE_HEIGHT_FRAC 0.5f   // panel covers the top half of the screen

// how many scrollback rows fit given the panel height. exposed so input
// handling can page by a screenful.
int  console_visible_rows(int screen_h);

// draw the whole thing. no-op when closed (the hud handles the closed-state
// status peek separately, if it wants one).
void uiconsole_draw(const console_t *c, text_renderer *t, int sw, int sh);

#endif
