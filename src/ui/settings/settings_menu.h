#ifndef UI_SETTINGS_MENU_H
#define UI_SETTINGS_MENU_H

// the whole settings menu: a panel with a tab strip, the active tab's option
// list, and a footer of apply / revert / reset buttons. it owns the model, the
// tab strip, and one layout per tab (so each tab keeps its own scroll).
//
// the host drives it: open() on entry, build() once per ui frame inside the
// widget begin/end, and poll the result for "the user wants out" plus "live
// changed, go push it into the engine". nothing here touches gl — it only emits
// into the widget draw list.

#include "settings_model.h"
#include "settings_tabs.h"
#include "settings_layout.h"
#include "../widgets/widgets_context.h"

// what the host should do after a build. the menu folds its internal buttons up
// to this so the host never reaches into the menu's guts.
typedef enum {
    SETTINGS_RESULT_NONE = 0,
    SETTINGS_RESULT_CLOSE,     // back / escape: leave the menu
    SETTINGS_RESULT_APPLIED,   // live values changed; re-read them
} settings_result;

typedef struct {
    settings_model  model;
    settings_tabs   tabs;
    settings_layout layouts[SETTINGS_TAB_COUNT];

    // cached title text. could be a literal, but the host may want to localize.
    const char     *title;

    int open;                  // 1 while the menu is up
} settings_menu;

// one-time init: seeds the model from schema defaults and resets tab state.
void settings_menu_init(settings_menu *sm);

// (re)open the menu. snapshots the current live values as the revert baseline so
// the user can always bail back to where they started. picks the video tab.
void settings_menu_open(settings_menu *sm);

// close without an explicit apply. uncommitted edits are discarded (work is
// reset from baseline) so a later open starts clean.
void settings_menu_close(settings_menu *sm);

// build the menu inside `area` for one frame. returns what the host should do.
// edits land in model.work; on APPLIED the host reads model.live.
settings_result settings_menu_build(settings_menu *sm, wg_context *ctx,
                                    wg_rect area);

// convenience for the host: read a committed value after APPLIED.
const settings_value *settings_menu_value(const settings_menu *sm,
                                          settings_id id);

#endif
