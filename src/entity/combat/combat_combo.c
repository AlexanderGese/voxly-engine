#include "combat_combo.h"

// each link past the first adds this much to the damage multiplier.
#define COMBO_DMG_STEP    0.12f   // +12% per link, so a 5-chain caps ~1.48x
// each link shaves this fraction off the next cooldown ...
#define COMBO_CD_STEP     0.06f
// ... but never below this scale (cant just spam frame-perfect forever).
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
}

float combat_combo_land(combat_combo *cb, uint32_t target_id) {
    // if the window already lapsed (tick reset chain to 0) this starts fresh.
    if (cb->chain < COMBAT_COMBO_MAX) cb->chain++;
    cb->window      = COMBAT_COMBO_WINDOW;
    cb->last_target = target_id;
    cb->primed      = false;
    return combat_combo_mult(cb);
}

void combat_combo_miss(combat_combo *cb) {
    // a clean whiff drops the whole chain. you earned that.
    cb->chain  = 0;
    cb->window = 0.0f;
    cb->primed = false;
}

float combat_combo_mult(const combat_combo *cb) {
    int links = combo_clamp_links(cb->chain);
    if (links <= 1) return 1.0f;
    // links-1 because the first hit is the baseline 1.0x.
    return 1.0f + (float)(links - 1) * COMBO_DMG_STEP;
}

float combat_combo_cooldown_scale(const combat_combo *cb) {
    int links = combo_clamp_links(cb->chain);
    float scale = 1.0f - (float)links * COMBO_CD_STEP;
    if (scale < COMBO_CD_FLOOR) scale = COMBO_CD_FLOOR;
    return scale;
}

void combat_combo_tick(combat_combo *cb, float dt) {
    if (dt < 0.0f) dt = 0.0f;
    if (cb->chain <= 0) return;

    cb->window -= dt;
    if (cb->window <= 0.0f) {
        // window lapsed without a follow-up. chain over.
        cb->chain  = 0;
        cb->window = 0.0f;
        cb->primed = false;
    }
}

bool combat_combo_active(const combat_combo *cb) {
    return cb->chain > 0 && cb->window > 0.0f;
}
