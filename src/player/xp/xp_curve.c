#include "xp_curve.h"
#include "xp_config.h"
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
int64_t acc = 0;
for (int l = 0;
l < level;
l++)
        acc += xp_curve_to_next(l);
return acc;
int64_t into = total - base;
if (into < 0) into = 0;
int width = xp_curve_to_next(level);
if (into > width) into = width;
return (int)into;
}
