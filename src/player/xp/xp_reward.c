#include "xp_reward.h"

#include <stddef.h>

#include "xp_config.h"
#include "../../math/rng.h"

// deterministic per-level roll. we seed an rng from the level number so the
// same level always grants the same thing across saves and sessions. no
// stored reward history needed.
static void seed_for_level(rng *r, int level) {
    // mix the level a bit so adjacent levels don't produce correlated rolls.
    uint64_t s = (uint64_t)(level + 1) * 0x9E3779B97F4A7C15ull;
    s ^= s >> 29;
    rng_init(r, s ? s : 1);
}

// perk pool weighted toward the boring-but-useful stats early, fancier ones
// later. returns the stat and fills *amount.
static xp_stat roll_perk(rng *r, int level, float *amount) {
    // unlock fancier stats as you climb.
    int top = XP_STAT_COUNT;
    if (level < 5)  top = 2;   // only move_speed, reach early on
    if (level < 15 && top > 3) top = 3;

    xp_stat stat = (xp_stat)rng_range(r, 0, top - 1);

    // magnitude scales gently with level but stays modest. multipliers get
    // small deltas; additive stats get whole-ish numbers.
    switch (stat) {
    case XP_STAT_MOVE_SPEED: *amount = 0.03f + 0.002f * (level % 10); break;
    case XP_STAT_MINE_SPEED: *amount = 0.04f + 0.003f * (level % 10); break;
    case XP_STAT_MAGNET:     *amount = 0.05f + 0.004f * (level % 10); break;
    case XP_STAT_REACH:      *amount = 0.25f; break;
    case XP_STAT_MAX_HP:     *amount = (level % 2 == 0) ? 2.0f : 1.0f; break;
    default:                 *amount = 0.0f; break;
    }
    return stat;
}

xp_reward xp_reward_for_level(int level) {
    xp_reward rw;
    rw.kind = XP_RW_NONE;
    rw.level = level;
    rw.stat = XP_STAT_MOVE_SPEED;
    rw.amount = 0.0f;
    rw.heal = 0;

    if (level <= 0) return rw; // level 0 isn't a reward

    rng r;
    seed_for_level(&r, level);

    // milestones: every 10 is a big deal, every 5 a small one.
    if (level % 10 == 0) {
        rw.kind = XP_RW_MILESTONE;
        rw.heal = 6; // partial heal on the celebration
        rw.stat = roll_perk(&r, level, &rw.amount);
        // milestone perks are beefier.
        rw.amount *= 1.5f;
        return rw;
    }
    if (level % 5 == 0) {
        rw.kind = XP_RW_HEAL;
        rw.heal = 4;
        return rw;
    }

    // ordinary level: ~60% chance of a perk, else nothing (keeps perks from
    // saturating the slot set too fast).
    if (rng_float01(&r) < 0.6f) {
        rw.kind = XP_RW_PERK;
        rw.stat = roll_perk(&r, level, &rw.amount);
    }
    return rw;
}

int xp_reward_apply(const xp_reward *rw, xp_perk_set *perks, int *out_heal) {
    int granted_perk = 0;
    int heal = 0;

    switch (rw->kind) {
    case XP_RW_PERK:
        granted_perk = xp_perk_add(perks, rw->stat, rw->amount, rw->level);
        break;
    case XP_RW_HEAL:
        heal = rw->heal;
        break;
    case XP_RW_MILESTONE:
        granted_perk = xp_perk_add(perks, rw->stat, rw->amount, rw->level);
        heal = rw->heal;
        break;
    case XP_RW_NONE:
    default:
        break;
    }

    if (out_heal) *out_heal = heal;
    return granted_perk;
}
