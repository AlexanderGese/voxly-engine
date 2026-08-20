#ifndef UI_WIDGETS_CHECKBOX_H
#define UI_WIDGETS_CHECKBOX_H

// checkboxes and radio groups. a checkbox is a small square box + a caption to
// its right; the whole row is clickable, not just the box, because hitting a
// 12px box with a mouse is annoying and i refuse to ship that.

#include "widgets_context.h"
#include "widgets_layout.h"

// toggle *checked when clicked. returns 1 on the frame it flipped.
int wg_checkbox(wg_context *ctx, wg_layout *l, const char *label, int *checked);

// one option of a radio group. *selected holds the chosen index; clicking this
// row sets *selected = index. returns 1 if it became the selection this frame.
int wg_radio(wg_context *ctx, wg_layout *l, const char *label,
             int *selected, int index);

#endif
