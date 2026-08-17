#ifndef UI_HUD2_CROSSHAIR_H
#define UI_HUD2_CROSSHAIR_H

#include "hud2_batch.h"

// center crosshair with a little life: it spreads when you're mid-swing and
// flashes when a hit lands. four ticks + a center dot. no texture.

typedef enum {
    HUD2_CROSS_PLUS = 0,   // classic + with a gap
    HUD2_CROSS_DOT,        // single pixel dot
    HUD2_CROSS_T,          // bottom-less plus (for placing)
} hud2_cross_style;

typedef struct {
    hud2_cross_style style;
    float gap;             // base gap from center to each tick (px)
    float len;            // tick length (px)
    float thick;          // tick thickness (px)

    float spread;         // current extra gap from recoil, decays to 0
    float hit_flash;      // 0..1 flash amount, decays
    int   targeting;      // is the player aimed at a block this frame
} hud2_crosshair;

void hud2_crosshair_init(hud2_crosshair *c);

// nudge the spread out (e.g. on swing). amount in px.
void hud2_crosshair_recoil(hud2_crosshair *c, float amount);
// trigger the confirm flash (e.g. block broken / hit registered).
void hud2_crosshair_hit(hud2_crosshair *c);

void hud2_crosshair_update(hud2_crosshair *c, int targeting, float dt);
void hud2_crosshair_draw(hud2_crosshair *c, hud2_batch *b, int sw, int sh);

#endif
