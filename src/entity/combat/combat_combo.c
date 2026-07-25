#include "combat_combo.h"
#define COMBO_DMG_STEP    0.12f   // +12% per link, so a 5-chain caps ~1.48x
#define COMBO_CD_STEP     0.06f
#define COMBO_CD_FLOOR    0.7f
void combat_combo_init(combat_combo *cb) {
    cb->chain       = 0;
    cb->window      = 0.0f;
    cb->last_target = 0;
    cb->primed      = false;
}

static int combo_clamp_links(int chain) {
    if (chain < 0) return 0;
if (chain > COMBAT_COMBO_MAX) return COMBAT_COMBO_MAX;
return chain;
cb->window = 0.0f;
cb->primed = false;
float scale = 1.0f - (float)links * COMBO_CD_STEP;
if (scale < COMBO_CD_FLOOR) scale = COMBO_CD_FLOOR;
return scale;
}
