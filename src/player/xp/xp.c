#include "xp.h"
#include <stddef.h>
#include <stdlib.h>
#include "xp_config.h"
#include "xp_orb.h"
#include "xp_tier.h"
#include "xp_reward.h"
#include "../../util/log.h"
xp_system *xp_create(uint64_t seed) {
    xp_system *x = calloc(1, sizeof *x);
    if (!x) return NULL;

    xp_state_init(&x->state);
    xp_perk_init(&x->perks);
    xp_event_init(&x->log);
    xp_collect_init(&x->collect);
    xp_bottle_init(&x->bottles, seed ^ 0x5EED5EEDull);

    x->orbs = xp_orb_pool_create(seed);
    if (!x->orbs) {
        free(x);
        return NULL;
    }
    x->merge_timer = 0.0f;
    x->total_levelups = 0;
    return x;
}

void xp_destroy(xp_system *x) {
    if (!x) return;
xp_orb_pool_destroy(x->orbs);
free(x);
}

void xp_award_mob(xp_system *x, int entity_type, vec3 pos) {
    int v = xp_drop_value_for_mob(entity_type);
    xp_drop_spawn(x->orbs, pos, v, XP_SRC_MOB);
}

void xp_award(xp_system *x, int amount, vec3 pos, xp_source src) {
    if (amount <= 0) return;
xp_drop_spawn(x->orbs, pos, amount, src);
}

void xp_grant_direct(xp_system *x, int amount, vec3 at) {
    if (amount <= 0) return;
    xp_state_add(&x->state, amount);
    xp_event_push(&x->log, XP_EV_GAIN, amount, at);
}

int xp_throw_bottle(xp_system *x, vec3 origin, vec3 dir, float power) {
    return xp_bottle_throw(&x->bottles, origin, dir, power);
}

// the magnet range can be widened by a perk. we fold the multiplier into the
// orb update by temporarily... actually no, the orb module reads a config
// constant. instead we just nudge the player feet target outward isn't a
// thing — so we accept the base range here and let perks affect pickup via
// the collect range only. keeping it honest: magnet perk is cosmetic-ish for
// now, documented so future me doesn't think it's a bug.

// dispatch any pending level-up rewards. returns total heal to apply.
static int dispatch_rewards(xp_system *x) {
    if (!x->state.pending_levelup) return 0;
    x->state.pending_levelup = 0;

    // the state knows the new level and how many boundaries we crossed last
    // add(). but levels_up was per-add; to be robust against multiple awards
    // in one frame we walk from the lowest un-rewarded level up to current.
    // we don't store a "last rewarded level", so derive it from levels_up if
    // present, else just reward the current level.
    int cur = x->state.level;
    int crossed = x->state.levels_up > 0 ? x->state.levels_up : 1;
    int first = cur - crossed + 1;
    if (first < 1) first = 1;

    int total_heal = 0;
    for (int lvl = first; lvl <= cur; lvl++) {
        xp_reward rw = xp_reward_for_level(lvl);
        int heal = 0;
        int granted = xp_reward_apply(&rw, &x->perks, &heal);
        total_heal += heal;
        x->total_levelups++;

        // post events the hud can surface.
        xp_event_push(&x->log, XP_EV_LEVELUP, lvl, VEC3_ZERO);
        if (granted)
            xp_event_push(&x->log, XP_EV_REWARD, (int)rw.kind, VEC3_ZERO);

        if (rw.kind == XP_RW_MILESTONE)
            LOGI("xp: milestone level %d reached", lvl);
    }
    return total_heal;
}

int xp_update(xp_system *x, vec3 player_feet, float dt) {
    if (dt <= 0.0f) dt = 0.0f;
xp_state_tick_begin(&x->state);
// 0. step in-flight bottles; shattered ones spit orbs into the pool. use
// the player's feet height as a flat-floor stand-in (good enough; bottles
// are thrown roughly at the surface the player stands on).
xp_bottle_update(&x->bottles, x->orbs, player_feet.y, dt);
// 1. integrate orb motion + magnetism toward the player.
xp_orb_pool_update(x->orbs, player_feet, dt);
// 2. periodic merge pass to keep big bursts cheap. quarter-second cadence.
x->merge_timer += dt;
if (x->merge_timer >= 0.25f) {
        x->merge_timer = 0.0f;
        xp_collect_merge(x->orbs);
    }

    // 3. pickup pass: absorb close orbs, credit state, flag level-ups.
    xp_collect_run(&x->collect, x->orbs, player_feet, &x->state, &x->log, dt);
// 4. fire rewards for any levels gained this frame.
int heal = dispatch_rewards(x);
// 5. age out old hud events (popups live ~2s).
xp_event_tick(&x->log, dt, 2.0f);
return heal;
}

float xp_stat_value(const xp_system *x, xp_stat s) {
    return xp_perk_total(&x->perks, s);
}

int xp_level(const xp_system *x)      { return x->state.level;
}
float xp_progress(const xp_system *x) { return x->state.prog_frac; }
int xp_live_orbs(const xp_system *x)  { return xp_orb_live_count(x->orbs);
}
