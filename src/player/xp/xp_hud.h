#ifndef PLAYER_XP_XP_HUD_H
#define PLAYER_XP_XP_HUD_H

// turns xp state + the event ring into screen-space draw data for the hud:
// the level number, the progress bar fill, and floating "+N" / "level up!"
// popups that rise and fade. the ui layer draws these; we just lay them out.
//
// nothing here touches GL or fonts. it's pure layout over a known viewport,
// so it's trivially testable and the actual draw stays in the render layer.

#include "../../math/vec2.h"
#include "../../math/vec3.h"
#include "xp.h"

#define XP_HUD_MAX_POPUPS  16

typedef enum {
    XP_POPUP_GAIN,    // "+5"
    XP_POPUP_LEVEL,   // "level 12!"
    XP_POPUP_REWARD,  // "perk unlocked"
} xp_popup_kind;

typedef struct {
    xp_popup_kind kind;
    vec2  pos;        // screen px, origin top-left
    float alpha;      // 0..1, faded by age
    float scale;      // pops in then settles
    int   value;      // amount or level, depends on kind
} xp_hud_popup;

typedef struct {
    // the level + bar, in screen px for a given viewport.
    int    level;
    vec2   bar_pos;     // top-left of the bar
    vec2   bar_size;    // full bar dimensions
    float  bar_fill;    // 0..1, how much to draw filled

    // floating popups, freshest last.
    xp_hud_popup popups[XP_HUD_MAX_POPUPS];
    int    popup_count;
} xp_hud_layout;

// build the full hud layout for `viewport` (px). `popup_life` is the seconds a
// popup lives, matched to the event ring's max_age so they appear/vanish in
// sync. reads, never mutates, the system.
void xp_hud_build(const xp_system *x, vec2 viewport, float popup_life,
                  xp_hud_layout *out);

#endif
