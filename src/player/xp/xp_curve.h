#ifndef PLAYER_XP_XP_CURVE_H
#define PLAYER_XP_XP_CURVE_H
// level <-> experience conversions.
//
// "xp_to_next" is how much you need to clear the *current* level.
// "xp_total_for" is the cumulative xp at the start of a given level.
// both are integer; xp is never fractional in this engine.
#include <stdint.h>
int       xp_curve_to_next(int level);
int64_t   xp_curve_total_for(int level);
int       xp_curve_level_at(int64_t total, float *progress_out);
int       xp_curve_progress_in(int64_t total, int level);
#endif
