#ifndef PLAYER_EFFECTS_EFFECTS_HUD_H
#define PLAYER_EFFECTS_EFFECTS_HUD_H
#include "effects_set.h"
#include "../../math/vec3.h"
// the read-only view the inventory / hud screen draws. snapshots the live set
// into a sorted, display-ready list so the renderer isn't poking at gameplay
// state mid-frame and so we can sort/format without touching the source.
//
// no GL in here either — it produces strings + numbers + a tint, and the actual
// quad-drawing lives in the render layer that consumes this.
typedef struct {
    effects_kind kind;
    const char  *name;       // from the def, ready to print
    int   level;             // 1-based, for the "II" / "III" suffix
    int   seconds_left;      // -1 == infinite (shows as a glyph, not a number)
    float fraction;          // 0..1 duration remaining, for a progress bar
    bool  fading;            // < 5s and finite: the icon should blink
    bool  beneficial;        // beneficial effects render on a different row
    vec3  tint;              // the effect's swatch colour, 0..1
} effects_hud_entry;
#define EFFECTS_HUD_MAX  EFFECTS_MAX_ACTIVE
typedef struct {
    effects_hud_entry entries[EFFECTS_HUD_MAX];
    int count;
} effects_hud_list;
// build the list from a set. `tps` converts the tick durations to seconds for
// display (20 for the default loop). buffs are listed before debuffs, and
// within each group the soonest-to-expire comes first so the player sees what's
// about to drop. returns list->count.
int effects_hud_build(effects_hud_list *list, const effects_set *s, float tps);
// roman-ish level label ("I".."VIII"); falls back to a number past the table.
const char *effects_hud_level_label(int level);
// pretty mm:ss into `buf` (size >= 8). infinite (seconds < 0) writes a glyph.
void effects_hud_format_time(int seconds_left, char *buf, int cap);
#endif
