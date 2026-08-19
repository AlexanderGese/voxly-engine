#ifndef UI_SETTINGS_LAYOUT_H
#define UI_SETTINGS_LAYOUT_H

// lays out the option rows for one tab and drives each control against the
// model's working copy. each row is "label on the left, control on the right",
// with a dirty marker drawn when the field differs from the running value.
//
// owns a tiny bit of per-tab scroll state because the control tab in particular
// can outgrow a short panel. handed the content rect; clips to it.

#include "settings_model.h"
#include "settings_types.h"
#include "../widgets/widgets_context.h"

typedef struct {
    float scroll;        // pixels scrolled, >= 0
    float content_h;     // last frame's measured content height, for clamping
} settings_layout;

void settings_layout_init(settings_layout *sl);

// reset scroll to top. called when the tab changes so a new tab starts at the
// top instead of inheriting the previous tab's offset.
void settings_layout_reset(settings_layout *sl);

// build every option under `tab` into `content`. edits go to the model's work
// copy; returns the number of fields that changed this frame (0 if none). does
// NOT call settings_model_refresh — the menu does that once after all tabs.
int settings_layout_build(settings_layout *sl, wg_context *ctx, wg_rect content,
                          settings_model *m, settings_tab tab);

#endif
