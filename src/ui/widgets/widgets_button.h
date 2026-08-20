#ifndef UI_WIDGETS_BUTTON_H
#define UI_WIDGETS_BUTTON_H

// clickable widgets: plain buttons, toggle buttons, and an icon-less repeat
// button for +/- steppers. all built on wg_behavior so they share the exact
// same hot/active rules as everything else — no bespoke click handling.

#include "widgets_context.h"
#include "widgets_layout.h"

// draw a button filling rect `r` with caption `label`. returns 1 on a completed
// click (press + release both inside). the low-level form, used by the layout
// wrappers below and by anyone placing buttons by hand (toolbars, dialogs).
int wg_button_rect(wg_context *ctx, wg_id id, wg_rect r, const char *label);

// consume a layout row and place a button in it. id is hashed from the label.
int wg_button(wg_context *ctx, wg_layout *l, const char *label);

// half-width button, for "ok / cancel" style pairs you lay out yourself.
int wg_button_frac(wg_context *ctx, wg_layout *l, float frac, const char *label);

// a toggle button: stays lit while *on is set, flips *on when clicked, returns
// 1 on the frame it changed. think "wireframe: on" debug switches.
int wg_toggle(wg_context *ctx, wg_layout *l, const char *label, int *on);

// a stepper: "< label >" with two arrow hit-zones. returns -1 / +1 when an
// arrow is clicked, 0 otherwise. used for enum/int settings that cycle.
int wg_stepper(wg_context *ctx, wg_layout *l, const char *label);

#endif
