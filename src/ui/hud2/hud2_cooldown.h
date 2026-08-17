#ifndef UI_HUD2_COOLDOWN_H
#define UI_HUD2_COOLDOWN_H

#include "hud2_batch.h"

// the attack-cooldown indicator: a little arc that sweeps around just under the
// crosshair while your swing recharges, then pops to full and fades. drawn as a
// fan of thin triangles since the batcher has no native arc. when fully charged
// it stays hidden so it isnt visual noise during normal play.

#define HUD2_COOLDOWN_SEGS  24    // arc resolution

typedef struct {
    float charge;        // 0..1, 1 = ready to swing
    float full_flash;    // 0..1 pop when it hits full, decays
    int   was_full;      // edge detect for the flash
    int   inited;
} hud2_cooldown;

void hud2_cooldown_init(hud2_cooldown *cd);

// reset to 0 (just swung). it'll recharge over `recover` seconds via update.
void hud2_cooldown_trigger(hud2_cooldown *cd);

// advance the recharge. recover is the full 0..1 recharge time in seconds.
void hud2_cooldown_update(hud2_cooldown *cd, float recover, float dt);

// draw centered at (cx,cy), radius r. it draws nothing while idle-at-full.
void hud2_cooldown_draw(hud2_cooldown *cd, hud2_batch *b,
                        float cx, float cy, float r);

#endif
