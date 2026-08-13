#include "tools_haste.h"

dig_state tools_haste_state_default(void) {
    dig_state st;
    st.head_submerged = 0;
    st.feet_on_ground = 1;
    st.aqua_affinity  = 0;
    st.haste_level    = 0;
    st.fatigue_level  = 0;
    return st;
}

// each haste level is a flat +20% to dig speed. multiplicative across levels so
// haste II is 1.2*1.2 = 1.44x, matching how the buff actually stacks per-tick.
float tools_haste_mult(int haste_level) {
    if (haste_level <= 0) return 1.0f;
    float m = 1.0f;
    for (int i = 0; i < haste_level; i++) m *= 1.20f;
    return m;
}

// mining fatigue is brutal: each level multiplies the *time* by ~3.33, i.e.
// divides speed by that. we express it as a speed-ish fatigue < 1 so the env
// can fold it the same way it folds haste.
float tools_haste_fatigue_mult(int fatigue_level) {
    if (fatigue_level <= 0) return 1.0f;
    float m = 1.0f;
    for (int i = 0; i < fatigue_level; i++) m *= 0.30f;   // 1/3.33
    return m;
}

dig_env tools_haste_env(const dig_state *st) {
    dig_env e = tools_env_default();

    // underwater penalty only bites if your head is actually under and you don't
    // have aqua affinity to cancel it.
    e.underwater = (st->head_submerged && !st->aqua_affinity) ? 1 : 0;

    // airborne digging penalty. mirrors the engine's on_ground flag.
    e.on_ground  = st->feet_on_ground ? 1 : 0;

    e.haste   = tools_haste_mult(st->haste_level);
    e.fatigue = tools_haste_fatigue_mult(st->fatigue_level);
    return e;
}
