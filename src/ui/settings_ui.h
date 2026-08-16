#ifndef UI_SETTINGS_UI_H
#define UI_SETTINGS_UI_H

#include "../render/gl.h"
#include "../game/settings.h"

// simple text-list settings menu. up/down to navigate, left/right to
// change values, enter to apply, escape to close.

typedef struct {
    int visible;
    int cursor;
    int num_entries;
} settings_ui;

void settings_ui_init(settings_ui *su);
void settings_ui_toggle(settings_ui *su);
void settings_ui_up(settings_ui *su);
void settings_ui_down(settings_ui *su);
void settings_ui_change(settings_ui *su, game_settings *gs, int delta);
void settings_ui_draw(const settings_ui *su, const game_settings *gs,
                      glid prog, int sw, int sh);

#endif
