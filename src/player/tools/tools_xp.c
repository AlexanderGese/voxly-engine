#include "tools_xp.h"

// cost curve: cheap early, ramps up. piecewise like the real game so the first
// handful of levels come quick and then it grinds.
int tools_xp_for_level(int level) {
    if (level < 0) level = 0;
    if (level < 16)  return 2 * level + 7;
    if (level < 31)  return 5 * level - 38;
    return 9 * level - 158;
}

// recompute level + pool from the lifetime total. linear walk, but levels are
// small numbers so this never costs anything worth caring about.
static void recompute(xp_state *xs) {
    int level = 0;
    int remaining = xs->total;
    while (1) {
        int need = tools_xp_for_level(level);
        if (remaining < need) break;
        remaining -= need;
        level++;
    }
    xs->level   = level;
    xs->pool    = remaining;
    xs->to_next = tools_xp_for_level(level);
}

void tools_xp_init(xp_state *xs) {
    xs->total   = 0;
    xs->level   = 0;
    xs->pool    = 0;
    xs->to_next = tools_xp_for_level(0);
}

int tools_xp_add(xp_state *xs, int amount) {
    if (amount <= 0) return 0;
    int before = xs->level;
    xs->total += amount;
    recompute(xs);
    int gained = xs->level - before;
    return gained > 0 ? gained : 0;
}

int tools_xp_spend(xp_state *xs, int amount) {
    if (amount <= 0) return 1;
    if (xs->total < amount) return 0;
    xs->total -= amount;
    recompute(xs);
    return 1;
}

float tools_xp_bar_frac(const xp_state *xs) {
    if (xs->to_next <= 0) return 0.0f;
    float f = (float)xs->pool / (float)xs->to_next;
    if (f < 0.0f) f = 0.0f;
    if (f > 1.0f) f = 1.0f;
    return f;
}
