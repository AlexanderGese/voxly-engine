#ifndef UI_SETTINGS_TYPES_H
#define UI_SETTINGS_TYPES_H

// shared vocabulary for the settings menu. this subsystem is a self-contained
// tabbed settings panel that rides on top of the immediate-mode widget layer
// (src/ui/widgets): it owns a schema of typed options, three copies of the value
// bag (live / working / baseline) and a small apply-revert state machine.
//
// it deliberately does NOT touch the older game/settings.h struct directly — the
// host bridges the two at the edges. everything in here is screen-space pixels,
// top-left origin, same as the rest of ui/.

#include <stddef.h>
#include <stdint.h>

// the tabs across the top. order here is the order they paint. keep COUNT last,
// it sizes a couple of static tables.
typedef enum {
    SETTINGS_TAB_VIDEO = 0,
    SETTINGS_TAB_AUDIO,
    SETTINGS_TAB_CONTROL,
    SETTINGS_TAB_COUNT
} settings_tab;

// what kind of editor an option wants. the schema row carries one of these and
// the layout dispatches on it. enum options additionally carry a label list.
typedef enum {
    SETTINGS_OPT_FLOAT = 0,  // continuous slider, formatted "%.2f"
    SETTINGS_OPT_INT,        // stepped slider, integer readout
    SETTINGS_OPT_BOOL,       // on/off toggle
    SETTINGS_OPT_ENUM,       // pick-one cycler with named choices
} settings_opt_kind;

// stable identity for each setting. the serializer keys off the string name but
// the rest of the code keys off this id so renames stay cheap. keep COUNT last;
// it sizes the value bag.
typedef enum {
    // video
    SETTINGS_ID_RENDER_DISTANCE = 0,
    SETTINGS_ID_FOV,
    SETTINGS_ID_GAMMA,
    SETTINGS_ID_VSYNC,
    SETTINGS_ID_FANCY_GRAPHICS,
    SETTINGS_ID_VIEW_BOBBING,
    SETTINGS_ID_SMOOTH_LIGHTING,
    SETTINGS_ID_MAX_FPS,
    SETTINGS_ID_GUI_SCALE,
    // audio
    SETTINGS_ID_MASTER_VOLUME,
    SETTINGS_ID_MUSIC_VOLUME,
    SETTINGS_ID_SFX_VOLUME,
    SETTINGS_ID_SUBTITLES,
    // control
    SETTINGS_ID_MOUSE_SENS,
    SETTINGS_ID_INVERT_Y,
    SETTINGS_ID_TOGGLE_SPRINT,
    SETTINGS_ID_AUTO_JUMP,
    SETTINGS_ID_COUNT
} settings_id;

// names used in the on-disk key=value file. indexed by settings_id. kept in one
// place so the serializer and any debug dump agree.
const char *settings_id_key(settings_id id);

// human label shown in the menu. indexed by settings_id.
const char *settings_id_label(settings_id id);

// reverse lookup for the loader: name -> id, or SETTINGS_ID_COUNT if unknown.
settings_id settings_id_from_key(const char *key);

// the tab a given option lives under. used to filter the schema per tab.
settings_tab settings_id_tab(settings_id id);

// short title for a tab, e.g. "video". used by the tab strip.
const char *settings_tab_title(settings_tab t);

#endif
