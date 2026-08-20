#ifndef UI_WIDGETS_COMBO_H
#define UI_WIDGETS_COMBO_H

// dropdown / combo box. click the closed box to drop a list of options under
// it; click an option to pick it and close. immediate mode makes popups a bit
// fiddly — the open state has to persist, so like panels it lives in a tiny
// caller-owned struct.
//
// the dropped list is drawn after the rest of the panel's content (the caller
// emits combos last, or the list overlaps later widgets — there's no z-buffer
// here, draw order is z order). good enough for settings menus.

#include "widgets_context.h"
#include "widgets_layout.h"

typedef struct {
    int open;        // is the list dropped
    int hover_item;  // which option the mouse is over (-1 none)
} wg_combo;

static inline void wg_combo_init(wg_combo *c) { c->open = 0; c->hover_item = -1; }

// combo on a layout row. `items` is `count` nul-terminated strings, *sel is the
// chosen index. returns 1 the frame the selection changes.
int wg_combo_box(wg_context *ctx, wg_layout *l, const char *label, wg_combo *c,
                 const char *const *items, int count, int *sel);

#endif
