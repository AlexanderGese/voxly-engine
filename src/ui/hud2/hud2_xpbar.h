#ifndef UI_HUD2_XPBAR_H
#define UI_HUD2_XPBAR_H

#include "hud2_batch.h"

// the experience bar: a thin strip spanning above the hotbar with a level
// number floating over its center. fills left-to-right as you earn xp toward
// the next level, and does a green flash + brief overfill bounce when you ding.
// like the bars, it keeps a smoothed display value so gains animate.

typedef struct {
    int   level;            // current level
    float frac;             // 0..1 progress toward next level (target)
    float frac_disp;        // smoothed display fraction
    float ding;             // 0..1 level-up flash, decays
    int   last_level;       // to detect level-ups
    int   inited;
} hud2_xpbar;

void hud2_xpbar_init(hud2_xpbar *xp);

// xp is a running total; xp_for_level maps a level to the cumulative xp needed
// to reach it. we derive level + fraction from those.
void hud2_xpbar_set(hud2_xpbar *xp, int total_xp);

void hud2_xpbar_update(hud2_xpbar *xp, float dt);
void hud2_xpbar_draw(hud2_xpbar *xp, hud2_batch *b, int sw, float y);

// cumulative xp required to have reached `level`. exposed so callers and tests
// agree on the curve.
int  hud2_xp_for_level(int level);

// the level number to print + how green the ding flash currently is. text is
// drawn by the orchestrator.
int  hud2_xpbar_level(const hud2_xpbar *xp, float *out_ding);

#endif
