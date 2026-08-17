#ifndef UI_HUD2_BARS_H
#define UI_HUD2_BARS_H
#include "hud2_batch.h"
#include "../../game/health.h"
typedef struct {
    float hp_disp;       // smoothed 0..1 health
    float hp_ghost;      // trailing ghost, lags hp_disp on loss only
    float hunger_disp;   // smoothed 0..1 hunger
    float stam_disp;     // smoothed 0..1 stamina
    float pulse_t;       // free-running clock for the low-hp pulse
    int   inited;
} hud2_bars;
#endif
