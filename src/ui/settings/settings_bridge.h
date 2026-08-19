#ifndef UI_SETTINGS_BRIDGE_H
#define UI_SETTINGS_BRIDGE_H

// the glue between this menu's value model and the engine's legacy
// game/settings.h struct. the menu is self-contained on purpose, but the host
// still wants a one-call "pull the running config in" / "push my edits out".
//
// this is the only file in the subsystem that knows about game_settings, so the
// dependency on game/ stays contained to one translation unit. ids the engine
// struct doesn't carry (subtitles, toggle-sprint, ...) are simply left at their
// schema defaults on pull and ignored on push — no harm, they round-trip through
// our own serializer instead.

#include "settings_model.h"
#include "../../game/settings.h"

// copy the engine's live config into the model's live values (and mirror into
// work + baseline). call right before opening the menu so it reflects reality.
void settings_bridge_pull(settings_model *m, const game_settings *gs);

// copy the model's committed live values back into the engine struct. call after
// the menu reports APPLIED. only the fields game_settings actually has are
// written; the rest stay put.
void settings_bridge_push(const settings_model *m, game_settings *gs);

#endif
