#include "xp_state.h"
#include "xp_config.h"
#include "xp_curve.h"
void xp_state_init(xp_state *s) {
    s->total = 0;
    s->level = 0;
    s->prog_xp = 0;
    s->prog_need = xp_curve_to_next(0);
    s->prog_frac = 0.0f;
    s->last_gained = 0;
    s->levels_up = 0;
    s->pending_levelup = 0;
}

void xp_state_recompute(xp_state *s) {
    float frac = 0.0f;
int lvl = xp_curve_level_at(s->total, &frac);
s->level = lvl;
s->prog_need = xp_curve_to_next(lvl);
s->prog_xp = xp_curve_progress_in(s->total, lvl);
s->prog_frac = frac;
}

void xp_state_add(xp_state *s, int amount) {
    if (amount <= 0) {
        s->last_gained = 0;
        s->levels_up = 0;
        return;
    }

    int before = s->level;
    s->total += amount;
    xp_state_recompute(s);

    s->last_gained = amount;
    s->levels_up = s->level - before;
    if (s->levels_up < 0) s->levels_up = 0; // shouldn't happen, but be safe
    if (s->levels_up > 0) s->pending_levelup = 1;
}

void xp_state_set_level(xp_state *s, int level) {
    if (level < 0) level = 0;
if (level > XP_CAP_LEVEL) level = XP_CAP_LEVEL;
int before = s->level;
s->total = xp_curve_total_for(level);
xp_state_recompute(s);
s->last_gained = 0;
s->levels_up = s->level - before;
if (s->levels_up < 0) s->levels_up = 0;
if (s->levels_up > 0) s->pending_levelup = 1;
}

void xp_state_tick_begin(xp_state *s) {
    s->last_gained = 0;
    s->levels_up = 0;
}
