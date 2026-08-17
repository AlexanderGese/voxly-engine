#ifndef UI_HUD2_HOTBAR_H
#define UI_HUD2_HOTBAR_H

#include "hud2_batch.h"
#include "../../player/inventory.h"
#include "../../config.h"

// the bottom hotbar. nine slots, an animated selection box that slides between
// them, and a per-slot pop when the selection lands. block icons via the
// blockicon module. selection text (block name) fades in above on change.

typedef struct {
    float sel_x;                    // smoothed selection-box x (px), slides
    int   sel_idx;                  // selection we last saw
    float pop[HOTBAR_SLOTS];        // 0..1 pop timers per slot, decay
    float name_fade;                // 0..1, shows the selected block's name
    block_id last_block;            // to detect selection content changes
    int   inited;

    // geometry cache, recomputed each draw from screen size
    float slot;                     // slot edge size
    float gap;
    float origin_x;                 // x of slot 0
    float origin_y;
} hud2_hotbar;

void hud2_hotbar_init(hud2_hotbar *hb);
void hud2_hotbar_update(hud2_hotbar *hb, const inventory *inv,
                        int sw, int sh, float dt);
void hud2_hotbar_draw(hud2_hotbar *hb, hud2_batch *b, const inventory *inv);

// the name label is drawn by the orchestrator (it owns the text renderer).
// expose what to print and how visible it is.
const char *hud2_hotbar_label(const hud2_hotbar *hb, const inventory *inv,
                              float *out_alpha);

#endif
