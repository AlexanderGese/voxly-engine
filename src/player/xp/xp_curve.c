#include "xp_curve.h"

#include "xp_config.h"

// piecewise quadratic. three regimes, glued at the knees. i tuned these by
// hand until the early game felt brisk and the late game felt like a chore,
// which is apparently the design goal everywhere.

int xp_curve_to_next(int level) {
    if (level < 0) level = 0;
    if (level > XP_CAP_LEVEL) level = XP_CAP_LEVEL;

    if (level < XP_CURVE_KNEE_LO) {
        // 2L + 7
        return 2 * level + 7;
    } else if (level < XP_CURVE_KNEE_HI) {
        // 5L - 38
        return 5 * level - 38;
    } else {
        // 9L - 158
        return 9 * level - 158;
    }
}

int64_t xp_curve_total_for(int level) {
    if (level <= 0) return 0;
    if (level > XP_CAP_LEVEL) level = XP_CAP_LEVEL;

    // closed forms exist for each segment but the cap is tiny and this runs
    // approximately never, so just sum it. clarity over cleverness.
    int64_t acc = 0;
    for (int l = 0; l < level; l++)
        acc += xp_curve_to_next(l);
    return acc;
}

int xp_curve_level_at(int64_t total, float *progress_out) {
    if (total < 0) total = 0;

    int level = 0;
    int64_t consumed = 0;
    while (level < XP_CAP_LEVEL) {
        int step = xp_curve_to_next(level);
        if (consumed + step > total) break;
        consumed += step;
        level++;
    }

    if (progress_out) {
        int step = xp_curve_to_next(level);
        if (step <= 0) {
            *progress_out = 0.0f;
        } else {
            float p = (float)(total - consumed) / (float)step;
            if (p < 0.0f) p = 0.0f;
            if (p > 1.0f) p = 1.0f;
            *progress_out = p;
        }
    }
    return level;
}

int xp_curve_progress_in(int64_t total, int level) {
    int64_t base = xp_curve_total_for(level);
    int64_t into = total - base;
    if (into < 0) into = 0;
    // a level never holds more than its own width, but clamp anyway in case
    // someone hands us a mismatched (total, level) pair.
    int width = xp_curve_to_next(level);
    if (into > width) into = width;
    return (int)into;
}
