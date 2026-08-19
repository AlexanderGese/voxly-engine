#ifndef UI_MENUS_TYPES_H
#define UI_MENUS_TYPES_H

// shared little types for the menu subsystem. the menus sit on top of the
// immediate-mode widget layer (src/ui/widgets) and drive a small screen stack.
// nothing in here touches gl — the host drains the widget draw list like it does
// for the rest of the hud.
//
// screen-space, pixels, top-left origin, same as everything else in ui/.

#include <stdint.h>

// the concrete screens we know how to build. the stack stores these and the
// manager dispatches on them. keep MENUS_SCREEN_COUNT last.
typedef enum {
    MENUS_SCREEN_NONE = 0,   // sentinel, "no screen" / in-game
    MENUS_SCREEN_MAIN,       // title screen
    MENUS_SCREEN_PAUSE,      // esc-in-world pause
    MENUS_SCREEN_SETTINGS,   // sliders + toggles
    MENUS_SCREEN_KEYBINDS,   // rebind list w/ capture
    MENUS_SCREEN_COUNT
} menus_screen_id;

// what a screen wants the host to do after a frame. the manager folds the
// per-screen result up to the caller (game loop) so menus never call back into
// the game directly — keeps the dependency arrow pointing one way.
typedef enum {
    MENUS_ACT_NONE = 0,
    MENUS_ACT_RESUME,        // close menus, back to world
    MENUS_ACT_NEW_WORLD,
    MENUS_ACT_LOAD_WORLD,
    MENUS_ACT_SAVE,
    MENUS_ACT_QUIT,          // exit the whole game
    MENUS_ACT_BACK,          // pop one screen
    MENUS_ACT_OPEN_SETTINGS,
    MENUS_ACT_OPEN_KEYBINDS,
    MENUS_ACT_APPLY_SETTINGS, // settings changed, host should re-read
} menus_action;

// a single navigable item index. screens build a list of these each frame and
// the nav kernel walks them. -1 means "nothing focused yet".
typedef int menus_item;
#define MENUS_NO_ITEM (-1)

// runtime knobs the settings screen edits. this is a plain value bag the host
// owns; the menu just reads/writes it and raises APPLY when the user commits.
// ranges are documented inline because nobody remembers them otherwise.
typedef struct {
    float master_volume;   // 0..1
    float music_volume;    // 0..1
    float mouse_sens;      // 0.02..0.5, radians per pixel-ish
    int   render_distance; // 2..16 chunks
    int   fov;             // 60..110 degrees
    int   vsync;           // 0/1
    int   invert_y;        // 0/1
    int   show_fps;        // 0/1
    float gamma;           // 0.5..2.0
} menus_settings;

void menus_settings_defaults(menus_settings *s);

// clamp every field back into its legal range. called after edits so a stray
// drag can't leave fov at 4000.
void menus_settings_clamp(menus_settings *s);

// true if any field differs. used to decide whether the apply button is live.
int  menus_settings_differs(const menus_settings *a, const menus_settings *b);

#endif
