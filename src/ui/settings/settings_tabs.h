#ifndef UI_SETTINGS_TABS_H
#define UI_SETTINGS_TABS_H

// the tab strip across the top of the settings panel. owns which tab is active
// plus a little hover/underline animation phase so switching feels less abrupt.
// it draws itself into the widget draw list and reports clicks back as a tab id.

#include "settings_types.h"
#include "../widgets/widgets_context.h"

typedef struct {
    settings_tab active;
    settings_tab hovered;     // SETTINGS_TAB_COUNT when none

    // 0..1 animation phase of the active underline sliding toward the active
    // tab. we lerp this each frame; purely cosmetic, the logic uses `active`.
    float        underline_t;
    float        underline_from;   // normalized x at animation start
    float        underline_to;     // normalized x of the active tab
} settings_tabs;

void settings_tabs_init(settings_tabs *t);

// switch tabs programmatically (keyboard tab/shift-tab). clamps + retriggers the
// underline slide. returns 1 if the active tab actually moved.
int  settings_tabs_set(settings_tabs *t, settings_tab which);
int  settings_tabs_next(settings_tabs *t, int dir);

// build + draw the strip in `bar`. handles hover/click and updates the active
// tab. returns the now-active tab (== t->active). advances animation by ctx->dt.
settings_tab settings_tabs_build(settings_tabs *t, wg_context *ctx, wg_rect bar);

#endif
