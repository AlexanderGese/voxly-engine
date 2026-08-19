#ifndef UI_SETTINGS_H
#define UI_SETTINGS_H

// umbrella include for the tabbed settings menu (video / audio / control tabs,
// sliders + toggles + cyclers, apply / revert / defaults). pull this in from the
// host and you get the whole subsystem.
//
// typical host flow:
// settings_menu_init(&menu);          // once
// 
// settings_bridge_pull(&menu.model, gs);
// settings_menu_open(&menu);          // on entry
// 
// // each ui frame, inside wg_begin/wg_end:
// settings_result r = settings_menu_build(&menu, ctx, panel_rect);
// if (r == SETTINGS_RESULT_APPLIED) settings_bridge_push(&menu.model, gs);
// if (r == SETTINGS_RESULT_CLOSE)   /* pop the screen */;
//
// the layers underneath (model / schema / value / serialize) are usable on their
// own if a host wants a headless settings store with no ui.

#include "settings_types.h"
#include "settings_value.h"
#include "settings_schema.h"
#include "settings_model.h"
#include "settings_serialize.h"
#include "settings_diff.h"
#include "settings_panel.h"
#include "settings_slider.h"
#include "settings_tabs.h"
#include "settings_layout.h"
#include "settings_menu.h"
#include "settings_bridge.h"

#endif
