#ifndef UI_MENUS_CONTROLS_H
#define UI_MENUS_CONTROLS_H

// menu-flavored controls. these are thin wrappers over the widget kernel
// (wg_behavior + the draw list) that *also* register themselves with the screen's
// nav ring, so the same control responds to both mouse and keyboard focus. the
// base widget layer stays input-agnostic; the keyboard-navigation glue lives here
// where the menus actually need it.
//
// every control takes the screen's menus_nav so it can:
// - register its id for the focus ring
// - draw a focus outline when it's the focused item
// - drain a pending dec/inc/activate intent from keyboard nav
//
// labels are short and the controls are full-width rows; this isn't a general ui
// toolkit, it's exactly what the three menus need.

#include "../widgets/widgets_context.h"
#include "../widgets/widgets_layout.h"
#include "menus_nav.h"

// a labeled button row. returns 1 the frame it's activated (mouse click or
// keyboard enter while focused). `enabled`=0 draws it dim and eats interaction.
int menus_ctl_button(wg_context *ctx, wg_layout *l, menus_nav *nav,
                     const char *label, int enabled);

// a left-label + right-value toggle. flips *value on activate. returns 1 if it
// changed this frame.
int menus_ctl_toggle(wg_context *ctx, wg_layout *l, menus_nav *nav,
                     const char *label, int *value);

// horizontal float slider with a grabbable handle. `*value` is edited in place,
// clamped to [lo,hi]. keyboard dec/inc step by `step`. returns 1 if the value
// changed this frame. the caption shows label on the left and the formatted value
// on the right.
int menus_ctl_slider(wg_context *ctx, wg_layout *l, menus_nav *nav,
                     const char *label, float *value, float lo, float hi,
                     float step);

// integer spinner: [label]   < value >  . left/right arrows or the on-screen
// chevrons step by 1 (held shift = by 5, read from ctx). clamps to [lo,hi].
// returns 1 if changed.
int menus_ctl_spinner(wg_context *ctx, wg_layout *l, menus_nav *nav,
                      const char *label, int *value, int lo, int hi);

// a non-interactive header row, slightly larger, used to break a settings panel
// into sections. doesn't touch nav.
void menus_ctl_header(wg_context *ctx, wg_layout *l, const char *text);

#endif
