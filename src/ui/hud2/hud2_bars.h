#ifndef UI_HUD2_BARS_H
#define UI_HUD2_BARS_H
#include "hud2_batch.h"
#include "../../game/health.h"
// health / hunger / stamina bars. they keep a smoothed display value so a
// chunk of damage animates down instead of teleporting, and there's a
// secondary "ghost" fill in white that lags behind to show recent loss
// (that little trailing bar a lot of games do).
typedef struct {
    float hp_disp;       // smoothed 0..1 health
    float hp_ghost;      // trailing ghost, lags hp_disp on loss only
    float hunger_disp;   // smoothed 0..1 hunger
    float stam_disp;     // smoothed 0..1 stamina
    float pulse_t;       // free-running clock for the low-hp pulse
    int   inited;
} hud2_bars;
void hud2_bars_init(hud2_bars *bars);
// feed it the survival snapshot each frame to advance the smoothing.
void hud2_bars_update(hud2_bars *bars, const survival *s, float dt);
// draw the three bars near the bottom-center, above the hotbar. base_y is the
// pixel y of the top bar; the hud composes that.
void hud2_bars_draw(hud2_bars *bars, hud2_batch *b,
                    int sw, int sh, float base_y);
#endif
